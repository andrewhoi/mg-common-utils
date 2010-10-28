all:repl4
repl4:repl4.c
	gcc -o repl4 repl4.c  `/usr/local/Percona-Server-5.1.50-rel11.4-111-Linux-i686/bin/mysql_config --libs` -I /root/pkg/Percona-Server/include/ -I /root/pkg/Percona-Server/sql -I /root/pkg/Percona-Server/regex
