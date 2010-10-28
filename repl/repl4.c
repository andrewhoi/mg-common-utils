#include <stdio.h>
#include <memory.h>
#include <stdlib.h>
#include <unistd.h>
#include <my_global.h>
#include <my_sys.h>
#include <m_string.h>
#include <mysql.h>
#include <mysql_embed.h>
#include <errmsg.h>
#include <my_getopt.h>
#include "sql_common.h"

int main(int argc,char **argv)
{
	MYSQL *m=mysql_init(NULL);
	if(!m) return -1;
	char *host="192.168.40.128";
	char *user="mg";
	char *pass="123qwe";
	char *db="test";
	unsigned short port=3306;
	if(!mysql_real_connect(m,host,user,pass,db,port,NULL,0))
	{
		printf("%s\n",mysql_error(m));
		return -2;
	}
	unsigned char buf[128];
	unsigned char *bufp=buf;
        memset(buf,0,sizeof(buf));
        int4store(buf,(uint32)4);	//start position
        int2store(buf+4,(uint32)0);	//flags
        char *logname="mysql-bin.000001";
        size_t len=strlen(logname);
        int4store(buf+6,(uint32)3);	//slave id
        memcpy(buf+10,logname,len);	//logname
        if(simple_command(m,COM_BINLOG_DUMP,buf,len+10,1))
	{
                return -3;
        	printf("%s\n",mysql_error(m));
	}
	for(;;)
	{
		size_t len=cli_safe_read(m);
		if(len < 8) break;
	}
	mysql_close(m);
	return 0;
}
