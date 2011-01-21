all:dump_innodb

dump_innodb: dump_innodb.c test0aux.c test0aux.h
	gcc -g -o dump_innodb dump_innodb.c test0aux.c -I. -I/opt/haildb/include -L/opt/haildb/lib -lhaildb -Wl,-rpath=/opt/haildb/lib
clean:
	rm -rf dump_innodb
