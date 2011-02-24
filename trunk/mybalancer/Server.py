
import sys, traceback, time, Ice
from mybalancer import *


Ice.loadSlice('Balancer.ice')
import Balancer

class myBalancerI(Balancer.myBalancer):
    def __init__(self):
        init_mc()
        add_server("test.ip_0",("10.210.74.152",3306,"mg","123qwe",None))
        add_server("test.ip_1",("10.210.74.152",3306,"mg","123qwe",None))
        add_server("test.ip_2",("10.210.74.152",3306,"mg","123qwe",None))
    def sqlInsert(self, sql, db, table, hashkey, hashval, current=None):
        return sql_insert(sql,db,table,hashkey,hashval)
    def sqlDelete(self, sql, db, table, hashkey, hashval, current=None):
        return sql_delete(sql,db,table,hashkey,hashval)
    def sqlUpdate(self, sql, db, table, hashkey, hashval, current=None):
        return sql_update(sql,db,table,hashkey,hashval)
    def sqlSelect(self, sql, db, table, hashkey, hashval, expire, current=None):
        return sql_select(sql,db,table,hashkey,hashval,expire)

class Server(Ice.Application):
    def run(self, args):
        adapter = self.communicator().createObjectAdapter("Balancer")
        adapter.add(myBalancerI(), self.communicator().stringToIdentity("balancer"))
        adapter.activate()
        self.communicator().waitForShutdown()
        return 0

sys.stdout.flush()
app = Server()
sys.exit(app.main(sys.argv,"config.server"))
