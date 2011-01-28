#include <ehaildb.h>
#include <haildb.h>
#include <string>
#include <map>
#include <stdlib.h>
#include <iostream>

using namespace std;

bool ehaildb::init() {
    ib_err_t err;
    err=ib_init();
    if(err != DB_SUCCESS){
        puts(ib_strerror(err));
        return false;
    }
    err = ib_cfg_set_int("flush_log_at_trx_commit", 2);
    if(err != DB_SUCCESS){
        cerr << ib_strerror(err) << endl;
        return false;
    }
    err = ib_cfg_set_int("log_file_size", 5*1024*1024);
    if(err != DB_SUCCESS){
        cerr << ib_strerror(err) << endl;
        return false;
    }
    err = ib_cfg_set_int("log_files_in_group", 2);
    if(err != DB_SUCCESS){
        cerr << ib_strerror(err) << endl;
        return false;
    }
    err = ib_cfg_set_text("log_group_home_dir", "./");
    if(err != DB_SUCCESS){
        cerr << ib_strerror(err) << endl;
        return false;
    }
    err = ib_cfg_set_text("data_home_dir", "./");
    if(err != DB_SUCCESS){
        cerr << ib_strerror(err) << endl;
        return false;
    }
    err = ib_cfg_set_text("data_file_path", "ibdata1:10M:autoextend");
    if(err != DB_SUCCESS){
        cerr << ib_strerror(err) << endl;
        return false;
    }
    err = ib_cfg_set_bool_on("file_per_table");
    if(err != DB_SUCCESS){
        cerr << ib_strerror(err) << endl;
        return false;
    }
    err=ib_startup("barracuda");
    if(err != DB_SUCCESS){
        cerr << ib_strerror(err) << endl;
        return false;
    }
    cerr << "DB init successed." << endl;
    return true;
}

bool ehaildb::deinit() {
    ib_err_t err;
    err=ib_shutdown(IB_SHUTDOWN_NORMAL);
    if(err != DB_SUCCESS){
        cerr << ib_strerror(err) << endl;
        return false;
    }
    return true;
}

bool ehaildb::create_database(string name) {
    ib_bool_t err;
    err=ib_database_create(name.c_str());
    if(err == IB_TRUE) {
        return true;
    } else {
        return false;
    }
}

bool ehaildb::create_table(string name) {
    ib_err_t err;
    ib_trx_t trx;
    ib_id_t tid=0;
    ib_tbl_sch_t tbl_sch=NULL;
    ib_idx_sch_t idx_sch=NULL;
    string tname("icq/");
    tname+=name;
    err=ib_table_schema_create(tname.c_str(),&tbl_sch,IB_TBL_COMPACT,0);
    if(err != DB_SUCCESS) {
        return false;
    }
    err=ib_table_schema_add_col(tbl_sch,"id",IB_INT,IB_COL_NOT_NULL,0,4);
    if(err != DB_SUCCESS) {
        return false;
    }
    err=ib_table_schema_add_col(tbl_sch,"data",IB_VARCHAR,IB_COL_NOT_NULL,0,65500);
    if(err != DB_SUCCESS) {
        return false;
    }
    err=ib_table_schema_add_index(tbl_sch,"PRIMARY_KEY",&idx_sch);
    if(err != DB_SUCCESS) {
        return false;
    }
    err=ib_index_schema_add_col(idx_sch,"id",0);
    if(err != DB_SUCCESS) {
        return false;
    }
    err=ib_index_schema_set_clustered(idx_sch);
    if(err != DB_SUCCESS) {
        return false;
    }
    trx=ib_trx_begin(IB_TRX_SERIALIZABLE);
    err=ib_schema_lock_exclusive(trx);
    err=ib_table_create(trx,tbl_sch,&tid);
    err=ib_schema_unlock(trx);
    err=ib_trx_commit(trx);
    ib_table_schema_delete(tbl_sch);
    return true;
}

bool ehaildb::drop_table(string name) {      
    ib_err_t err;
    ib_trx_t trx;

    string tname("icq/");
    tname+=name;

    trx=ib_trx_begin(IB_TRX_SERIALIZABLE);
    err=ib_schema_lock_exclusive(trx);
    err=ib_table_drop(trx,tname.c_str());
    err=ib_schema_unlock(trx);
    err=ib_trx_commit(trx);
    return true;
}

bool ehaildb::truncate_table(string name) {
    ib_err_t err;
    ib_id_t tid=0;
    err=ib_table_truncate(name.c_str(),&tid);
    if(err == DB_SUCCESS) {
        return true;
    } else {
        return false;
    }
}

bool ehaildb::drop_database(string name) {
    ib_err_t err;
    err=ib_database_drop(name.c_str());
    if(err == DB_SUCCESS) {
        return true;
    } else {
        return false;
    }
}

int ehaildb::insert(string name,string data) {
    ib_err_t err;
    ib_trx_t trx;
    ib_crsr_t crsr;
    trx=ib_trx_begin(IB_TRX_REPEATABLE_READ);
    string tname("icq/");
    tname+=name;
    err=ib_cursor_open_table(tname.c_str(),trx,&crsr);
    ib_tpl_t tpl;
    tpl=ib_clust_read_tuple_create(crsr);
    int pos=(this->tinfo[name].cur_id)++;
    err=ib_tuple_write_u32(tpl,0,pos);
    err=ib_col_set_value(tpl,1,data.c_str(),data.size());
    err=ib_cursor_insert_row(crsr,tpl);
    ib_tuple_delete(tpl);
    err=ib_cursor_close(crsr);
    err=ib_trx_commit(trx);
    return pos;
}

result ehaildb::get(string name,int pos) {
    ib_err_t err;
    ib_trx_t trx;
    ib_crsr_t crsr;
    trx=ib_trx_begin(IB_TRX_REPEATABLE_READ);
    string tname("icq/");
    tname+=name;
    err=ib_cursor_open_table(tname.c_str(),trx,&crsr);
    ib_tpl_t key;
    int res;
    key=ib_clust_search_tuple_create(crsr);
    err=ib_tuple_write_u32(key,0,pos);
    err=ib_cursor_moveto(crsr,key,IB_CUR_GE,&res);
    ib_tpl_t tpl;
    tpl=ib_clust_read_tuple_create(crsr);
    err=ib_cursor_read_row(crsr,tpl);
    int data_len=ib_col_get_len(tpl,1);
    const void *ptr;
    ptr=ib_col_get_value(tpl,1);
    result r;
    r.data=string((char *)ptr,data_len);
    r.pos=pos;
    err=ib_cursor_close(crsr);
    err=ib_trx_commit(trx);
    return r;
}

result ehaildb::get_next(string name,int pos) {
    ib_err_t err;
    ib_trx_t trx;
    ib_crsr_t crsr;
    trx=ib_trx_begin(IB_TRX_REPEATABLE_READ);
    string tname("icq/");
    tname+=name;
    err=ib_cursor_open_table(tname.c_str(),trx,&crsr);
    ib_tpl_t key;
    int res;
    key=ib_clust_search_tuple_create(crsr);
    err=ib_tuple_write_u32(key,0,pos);
    err=ib_cursor_moveto(crsr,key,IB_CUR_GE,&res);
    err=ib_cursor_next(crsr);
    ib_tpl_t tpl;
    tpl=ib_clust_read_tuple_create(crsr);
    err=ib_cursor_read_row(crsr,tpl);
    int data_len=ib_col_get_len(tpl,1);
    const void *ptr;
    ptr=ib_col_get_value(tpl,1);
    result r;
    r.data=string((char *)ptr,data_len);
    r.pos=pos;
    err=ib_cursor_close(crsr);
    err=ib_trx_commit(trx);
    return r;
}

result ehaildb::get_latest(string name) {
    ib_err_t err;
    ib_trx_t trx;
    ib_crsr_t crsr;
    trx=ib_trx_begin(IB_TRX_REPEATABLE_READ);
    string tname("icq/");
    tname+=name;
    err=ib_cursor_open_table(tname.c_str(),trx,&crsr);
    ib_tpl_t key;
    int res;
    err=ib_cursor_last(crsr);
    ib_tpl_t tpl;
    tpl=ib_clust_read_tuple_create(crsr);
    err=ib_cursor_read_row(crsr,tpl);
    int data_len=ib_col_get_len(tpl,1);
    const void *ptr;
    ptr=ib_col_get_value(tpl,1);
    int pos;
    err=ib_tuple_read_i32(tpl,0,&pos);
    
    result r;
    r.data=string((char *)ptr,data_len);
    r.pos=pos;
    err=ib_cursor_close(crsr);
    err=ib_trx_commit(trx);
    return r;
}

result ehaildb::get_oldest(string name) {
    ib_err_t err;
    ib_trx_t trx;
    ib_crsr_t crsr;
    trx=ib_trx_begin(IB_TRX_REPEATABLE_READ);
    string tname("icq/");
    tname+=name;
    err=ib_cursor_open_table(tname.c_str(),trx,&crsr);
    ib_tpl_t key;
    int res;
    err=ib_cursor_first(crsr);
    ib_tpl_t tpl;
    tpl=ib_clust_read_tuple_create(crsr);
    err=ib_cursor_read_row(crsr,tpl);
    int data_len=ib_col_get_len(tpl,1);
    const void *ptr;
    ptr=ib_col_get_value(tpl,1);

    int pos;
    err=ib_tuple_read_i32(tpl,0,&pos);
    
    result r;
    r.data=string((char *)ptr,data_len);
    r.pos=pos;
    err=ib_cursor_close(crsr);
    err=ib_trx_commit(trx);
    return r;
}

int main(int argc,char **argv){
    ehaildb *edb=new ehaildb();
    edb->init();
    edb->create_database("icq");
    bool ret;
    string name("mg");
    ret=edb->create_table(name);
    int pos;
    for(int i=1;i<1000;i++){
        pos=edb->insert(name,string(i,'a'));
    }
    
    result r;
    pos=100;
    r=edb->get(name,pos);
    cout << r.data << "\t" << r.pos << endl;
    
    r=edb->get_latest(name);
    cout << r.data << "\t" << r.pos << endl;

    r=edb->get_oldest(name);
    cout << r.data << "\t" << r.pos << endl;
    
    for(int i=1;i<100;i++) {
        r=edb->get_next(name,i);
        cout << r.data << "\t" << r.pos << endl;
    }
    
    
    ret=edb->drop_database("icq");
    delete edb;
}
