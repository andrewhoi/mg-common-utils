#include "ckwlib.h"

extern int used;
extern int rood;

extern node *nodes;
int main(int argc,char **argv)
{
	used=0;
	root=0;

	nodes=(node *)open_db(DB_FILE,DB_SIZE,1);
	//nodes=calloc(DB_SIZE,1);
	assert(nodes != NULL);
	FILE *fp=fopen(KW_FILE,"r");
	assert(fp != NULL);
	char line[KW_MAX_LEN];
	int strl=0;
	while(!feof(fp))
	{
		memset(line,0,KW_MAX_LEN);
		fgets(line,KW_MAX_LEN,fp);
		strl=strlen(line);
		if(strl == 0)
			continue;
		if(line[strl-1] == '\n')
			line[strl-1] = '\0';
		set_nodes(line);
	}
	fclose(fp);
	close_db(nodes,DB_SIZE);
	
	nodes=(node *)open_db(DB_FILE,DB_SIZE,0);
	char *s="adasdasda10086asdsa";
	int ret=check_keywords(s);
	printf("ckw %d\n",ret);
	
}
