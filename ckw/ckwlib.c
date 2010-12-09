#include "ckwlib.h"

int set_nodes(char *s)
{
	int p=0,rp=0;
	while( *s != '\0' )
	{
		rp=p;
		if(nodes[p].cell[(unsigned char)*s].point == 0 && *(s+1) != '\0' )
		//if(nodes[p].cell[(unsigned char)*s].point == 0 )
		{
			used++;
			nodes[p].cell[(unsigned char)*s].point=used;
			p=used;
		}
		else
		{
			p=nodes[p].cell[(unsigned char)*s].point;
		}
		s++;
	}
	nodes[rp].cell[(unsigned char)*(s-1)].end=1;
	return 0;
}

int get_nodes(char *s)
{
	int p=0;
	while( *s != '\0' )
	{
		if(nodes[p].cell[(unsigned char)*s].end == 1)
		{
			return 1;
		}
		else
		{
			p=nodes[p].cell[(unsigned char)*s].point;
		}
		s++;
	}
	return 0;

}

int get_nodes_rl(char *s,int len)
{
	int p=0;
	int i=1;
	while( len >= i-1 )
	{
		if(nodes[p].cell[(unsigned char)*s].end == 1)
		{
			return i;
		}
		else
		{
			p=nodes[p].cell[(unsigned char)*s].point;
			if(p == 0)
				return i;
		}
		s++;
		i++;
	}
	return i;

}

int check_keywords(char *s)
{
	int cr=0;
	while ( *s != '\0' )
	{
		cr=get_nodes(s);
		if(cr == 1)
		{
			return 1;
		}
		else
		{
			s++;
		}
	}
	return 0;
}

int check_keywords_n(char *s,int len)
{
	int cr=0;
	while ( len > 0 )
	{
		cr=get_nodes(s);
		if(cr == 1)
		{
			return 1;
		}
		else
		{
			s++;
		}
		len--;
	}
	return 0;
}



void *open_db(char *db_name,unsigned int size,int create)
{
	void *m;
	int fd,flag;
	if (create == 1) 
		flag=O_RDWR|O_CREAT|O_TRUNC;
	if (create == 0)	
		flag=O_RDWR;
	if((fd=open(db_name,flag,S_IRWXU)) < 0)
	{
		//perror("open");
		return NULL;
	}
	if(create == 1)
	{
		if(ftruncate(fd,size) != 0)
		{
			//perror("ftruncate");
			return NULL;
		}
	}
	if((m=mmap(0,size,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0)) == MAP_FAILED)
	{
		//perror("mmap");
		return NULL;
	}
	close(fd);
	return m;
}

void close_db(void *m,int size)
{
	if(m != NULL)
		munmap(m,size);
}
