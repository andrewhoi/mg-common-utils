#ifndef _EHAILDB_H_
#define _EHAILDB_H_

#include <string>
#include <map>
#include <pthread.h>

struct result {
    int pos;
    std::string data;
};
typedef struct result result;

struct info {
    int cur_id;
    pthread_mutex_t mutex;
};
typedef struct info info;   

class ehaildb {
public:
    ehaildb() {
        this->init();
        this->create_database("icq");
    };
    ~ehaildb() {
        this->deinit();
    };
    virtual bool init();
    virtual bool deinit();
    virtual bool create_database(std::string name);
    virtual bool create_table(std::string name);
    virtual bool drop_table(std::string name);
    virtual bool drop_database(std::string name);
    virtual bool truncate_table(std::string name);
    virtual int insert(std::string name,std::string data);
    virtual result get(std::string name,int pos);
    virtual result get_next(std::string name,int pos);
    virtual result get_latest(std::string name);
    virtual result get_oldest(std::string name);
private:
    std::map<std::string,info> tinfo;
};

#endif
