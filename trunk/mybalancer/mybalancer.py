import memcache
import MySQLdb
from hashlib import md5
import time
import Ice
import json

servers={}
mc=""

def add_server(table,server) :
    global servers
    servers[table]=server

def get_server(table) :
    global servers
    return servers.get(table)

def del_server(table) :
    global servers
    servers.pop(table)

def init_mc() :
    global mc
    mc=memcache.Client(["127.0.0.1:11211"])

def pack_sql(sql,db,table,hashkey,hashval) :
    hashslot=3
    hashr=memcache.crc32(str(hashval))
    slot=hashr % hashslot
    table = db + "." + table + "_" + str(slot)
    sql=sql.replace("/*table*/",table)
    #print sql
    return table,sql

def sql_execute(sql,table) :
    mycsr=get_cursor(table)
    mycsr.execute(sql)
    return (mycsr.rowcount,mycsr.fetchall())

def get_cursor(table) :
    host,port,user,passwd,csr=get_server(table)
    if csr != None :
        return csr
    else:
        conn=MySQLdb.connect(host=host,port=port,user=user,passwd=passwd)
        csr=conn.cursor()
        add_server(table,(host,port,user,passwd,csr))
        return csr

def lock_cache(key) :
    global mc
    return mc.add(key,0,1)

def unlock_cache(key) :
    global mc
    return mc.delete(key)

def get_from_cache(key) :
    global mc
    return mc.get(key)

def save_to_cache(key,val,expire=5) :
    global mc
    return mc.set(key,val,expire)

def sql_select(sql,db,table,hashkey,hashval,expire=1) :
    table,sql=pack_sql(sql,db,table,hashkey,hashval)
    cachekey=md5(sql).hexdigest()
    lockkey=cachekey + ".lock"
    if expire < 0 :
        return json.dumps(sql_execute(sql,table))
    i=0
    while i < 10:
        cache=get_from_cache(cachekey)
        if cache:
            return json.dumps(cache)
        else :
            lock=lock_cache(lockkey)
            if lock :
                res=sql_execute(sql,table)
                save_to_cache(cachekey,res,expire)
                unlock_cache(lockkey)
                return json.dumps(res)
            else :
                time.sleep(0.1)
                i=i+1
    return json.dumps(False)

def sql_common(sql,db,table,hashkey,hashval) :
    table,sql=pack_sql(sql,db,table,hashkey,hashval)
    res=sql_execute(sql,table)
    return json.dumps(res)

def sql_insert(sql,db,table,hashkey,hashval) :
    return sql_common(sql,db,table,hashkey,hashval)

def sql_update(sql,db,table,hashkey,hashval) :
    return sql_common(sql,db,table,hashkey,hashval)

def sql_delete(sql,db,table,hashkey,hashval) :
    return sql_common(sql,db,table,hashkey,hashval)


def test() :
    init_mc()
    add_server("test.ip_0",("10.210.74.152",3306,"mg","123qwe",None))
    add_server("test.ip_1",("10.210.74.152",3306,"mg","123qwe",None))
    add_server("test.ip_2",("10.210.74.152",3306,"mg","123qwe",None))
    sql="insert into /*table*/ (ip) values ( '/*val*/')"
    for i in range(1,10) :
        _sql=sql.replace("/*val*/",str(i))
        res=sql_insert(_sql,"test","ip","ip",i)
        print res
    sql="update /*table*/ set ip=ip*20 where ip=/*val*/"
    for i in range(1,10) :
        _sql=sql.replace("/*val*/",str(i))
        res=sql_update(_sql,"test","ip","ip",i)
        print res
    sql="select * from /*table*/ where ip=/*val*/"
    for i in range(1,10) :
        _sql=sql.replace("/*val*/",str(i*20))
        res=sql_select(_sql,"test","ip","ip",i,-1)
        print res
    sql="delete from /*table*/ where ip=/*val*/"
    for i in range(1,10) :
        _sql=sql.replace("/*val*/",str(i*20))
        res=sql_delete(_sql,"test","ip","ip",i)
        print res

if __name__ == "__main__" :
    test()
    

