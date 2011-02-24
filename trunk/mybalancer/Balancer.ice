module Balancer {
	interface myBalancer {
		string sqlInsert(string sql,string db,string table,string hashkey,string hashval);
		string sqlDelete(string sql,string db,string table,string hashkey,string hashval);
		string sqlUpdate(string sql,string db,string table,string hashkey,string hashval);
		string sqlSelect(string sql,string db,string table,string hashkey,string hashval,int expire);
	};
};

