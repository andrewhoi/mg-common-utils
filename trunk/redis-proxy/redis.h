#include <time.h>
#define READ_BUF_LEN 1024
#define WRITE_BUF_LEN 4096

struct RedisClient {
	int fd;
	char rbuf[READ_BUF_LEN];
	char wbuf[WRITE_BUF_LEN];
	char *extwbuf;
	int rlen;
	int wlen;
	int wpos;
	time_t lastinteraction;
};
typedef struct RedisClient redisClient;

typedef void redisCommandInit(void **privptr);
typedef void redisCommandProc(redisClient *c,void **privptr);
typedef void redisCommandDeinit(void **privptr);
