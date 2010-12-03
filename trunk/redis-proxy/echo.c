#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <poll.h>
#include "redis.h"
#include "anet.h"

#define CONN_ERR "-ERR: Connection error\r\n"
#define CONN_ERR_LEN 24

static int connectRedis() {
    int fd=anetTcpConnect(NULL, "localhost", 6379);
    return fd;
}

/*
return -1 if protocol error;
return 0 if we need more data;
return 1 if the request if full and valid

request example:

*3\r\n
$3\r\n
SET\r\n
$5\r\n
mykey\r\n
$7\r\n
myvalue\r\n

*/

int validSingleLineResponseBuffer(char *rbuf,int rlen) {
    if( rbuf[0] != '+' ) {
        return -1;
    }
    if( rlen < 3 || rbuf[rlen-2] != '\r'|| rbuf[rlen-1] != '\n' ) {
        return 0;
    }
    //rbuf++; // '+'
    char *p=strstr(rbuf,"\r\n");
    if(p - rbuf + 2 != rlen) {
        return -1;
    } else {
        return 1;
    }
    return -1;
}

int validErrorMsgResponseBuffer(char *rbuf,int rlen) {
    if( rbuf[0] != '-' ) {
        return -1;
    }
    if( rlen < 3 || rbuf[rlen-2] != '\r'|| rbuf[rlen-1] != '\n' ) {
        return 0;
    }
    //rbuf++; // '-'
    char *p=strstr(rbuf,"\r\n");
    if(p - rbuf + 2 != rlen) {
        return -1;
    } else {
        return 1;
    }
    return -1;
}

int validIntegerReplyResponseBuffer(char *rbuf,int rlen) {
    if( rbuf[0] != ':' ) {
        return -1;
    }
    if( rlen < 3 || rbuf[rlen-2] != '\r'|| rbuf[rlen-1] != '\n' ) {
        return 0;
    }
    //rbuf++; // ':'
    char *p=strstr(rbuf,"\r\n");
    if(p - rbuf + 2 != rlen) {
        return -1;
    } else {
        return 1;
    }
    return -1;
}

int validBulkReplyResponseBuffer(char *rbuf,int rlen) {
    char *op=rbuf;
    if(rbuf[0] != '$') {
        return -1;
    }
    if( rlen < 3 || rbuf[rlen-2] != '\r'|| rbuf[rlen-1] != '\n' ) {
        return 0;
    }
    rbuf++; // '$'
    char slen[11];
    int ilen;
    memset(slen,0,11);
    char *p=strstr(rbuf,"\r\n");
    strncpy(slen,rbuf,p-rbuf);
    rbuf=p;
    rbuf+=2; //"\r\n"
    ilen=atoi(slen);
    if(ilen == -1) {
        rbuf+=0;
    } else {
        rbuf+=ilen+2; // data and "\r\n"
    }
    
    if(rbuf - op > rlen) {
        return 0; // we need the next bulk.
    }
    if(rbuf - op == rlen) {
        return 1;
    }
    if(rbuf - op < rlen) {
        return -1;
    }
}

/*
*2\r\n$-1\r\n$3\r\n456\r\n
*/
int validMultiBuckResponseBuffer(char *rbuf,int rlen) {
    char *op=rbuf;
    if( rbuf[0] != '*' ) {
        return -1;
    }
    if( rlen < 3 || rbuf[rlen-2] != '\r'|| rbuf[rlen-1] != '\n' ) {
        return 0;
    }
    rbuf++; // '*'
    char slen[11];
    int argc,ilen;
    memset(slen,0,11);
    char *p=strstr(rbuf,"\r\n");
    strncpy(slen,rbuf,p-rbuf);
    rbuf=p;
    rbuf+=2; // "\r\n"
    argc=atoi(slen);
    if(argc < 0 || argc > 1024) {
        return -1;
    }
    if( argc == 0 && rbuf - op == rlen) {
        return 1; // we need more data.
    }
    if( argc != 0 && rbuf - op == rlen ) {
        return 0;
    }
    for(; argc > 0; argc--) {
        if(rbuf[0] != '$') {
            return -1;
        }
        rbuf++; // '$'
        memset(slen,0,11);
        p=strstr(rbuf,"\r\n");
        strncpy(slen,rbuf,p-rbuf);
        rbuf=p;
        rbuf+=2; //"\r\n"
        ilen=atoi(slen);
        if(ilen == -1) {
            rbuf+=0;
        } else {
            rbuf+=ilen+2; // data and "\r\n"
        }
        if(rbuf - op > rlen) {
            return 0; // we need the next bulk.
        }
        if(rbuf - op == rlen && argc != 1) { // not the last bulk
            return 0;
        }
    }
    if(rbuf - op != rlen) {
        return -1;
    } else {
        return 1;
    }
    return -1;   
}

int validResponseBuffer(char *rbuf,int rlen) {
    char s=rbuf[0];
    if(s == '+') {
        return validSingleLineResponseBuffer(rbuf,rlen);
    }
    if(s == '-') {
        return validErrorMsgResponseBuffer(rbuf,rlen);
    }
    if(s == ':') {
        return validIntegerReplyResponseBuffer(rbuf,rlen);
    }
    if(s == '$') {
        return validBulkReplyResponseBuffer(rbuf,rlen);
    }
    if(s == '*') {
        return validMultiBuckResponseBuffer(rbuf,rlen);
    }
    return -1;

}

void _redisCommandInit(void **privptr) {
    int *fd=calloc(sizeof(int),1);
    *fd=connectRedis();
    *privptr=(void *)fd;
    return;
}

void _redisCommandProc(redisClient *c,void **privptr) {
    int *fd=(int *)(*privptr);
    if(*fd == -1) {
        *fd=connectRedis();
        if(*fd == -1) {
            memset(c->wbuf,0,sizeof(c->wbuf));
            strcpy(c->wbuf,CONN_ERR);
            c->wlen=CONN_ERR_LEN;
            return;
        }
    }
    int size;
    struct pollfd pfd;
    pfd.fd=*fd;
    pfd.events = POLLOUT;
    if(poll(&pfd,1,100) > 0) {
        size=write(*fd, c->rbuf, c->rlen);
    } else {
        close(*fd);
        *fd=-1;
        memset(c->wbuf,0,sizeof(c->wbuf));
        strcpy(c->wbuf,CONN_ERR);
        c->wlen=CONN_ERR_LEN;
        return;
    }
    if(size != c->rlen) {
        close(*fd);
        *fd=-1;
        memset(c->wbuf,0,sizeof(c->wbuf));
        strcpy(c->wbuf,CONN_ERR);
        c->wlen=CONN_ERR_LEN;
        return;
    }
    pfd.events = POLLIN;
    if(poll(&pfd,1,100) > 0) {
        size=read(*fd,c->wbuf,WRITE_BUF_LEN);
    } else {
        close(*fd);
        *fd=-1;
        memset(c->wbuf,0,sizeof(c->wbuf));
        strcpy(c->wbuf,CONN_ERR);
        c->wlen=CONN_ERR_LEN;
        return;
    }
    if(size == -1 || size == 0 ) {
        close(*fd);
        *fd=-1;
        strcpy(c->wbuf,CONN_ERR);
        c->wlen=CONN_ERR_LEN;
        return;
    }
    c->wlen=size;
	int v=validResponseBuffer(c->wbuf,c->wlen);
	if(v == 1) {
	    return;
	}
	if(v == -1 || v == 0) {
        close(*fd);
        *fd=-1;
        memset(c->wbuf,0,sizeof(c->wbuf));
        strcpy(c->wbuf,CONN_ERR);
        c->wlen=CONN_ERR_LEN;
        return;
	}
    return;
}

void _redisCommandDeinit(void **privptr) {
    int *fd=(int *)*privptr;
    close(*fd);
    free(*privptr);
    return;
}
