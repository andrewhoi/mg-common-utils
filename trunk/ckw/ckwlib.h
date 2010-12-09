#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>


#define KW_FILE "kw2.txt"
#define DB_FILE "kw.db"
#define DB_SIZE 16384*512*32

#define KW_MAX_LEN 512

int set_nodes(char *s);
int get_nodes(char *s);
int get_nodes_rl(char *s,int len);
int check_keywords(char *s);
int check_keywords_n(char *s,int len);
void *open_db(char *db_name,unsigned int size,int create);
void close_db(void *m,int size);

typedef
struct _cell
{
	unsigned short point:15;
	unsigned short end:1;
}
cell;

typedef
struct _node
{
	 cell cell[256];
}
node;

node *nodes;

int used;
int root;
