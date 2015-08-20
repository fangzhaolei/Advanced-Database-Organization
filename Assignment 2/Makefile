all: test_assign2_1 test_assign2_2

test_assign2_1: test_assign2_1.o storage_mgr.o dberror.o buffer_mgr_stat.o buffer_mgr.o
	gcc test_assign2_1.o storage_mgr.o dberror.o buffer_mgr_stat.o buffer_mgr.o -o test_assign2_1

test_assign2_2: test_assign2_2.o storage_mgr.o dberror.o buffer_mgr_stat.o buffer_mgr.o
	gcc test_assign2_2.o storage_mgr.o dberror.o buffer_mgr_stat.o buffer_mgr.o -o test_assign2_2
	rm -rf *o

test_assign2_1.o: test_assign2_1.c
	gcc -c test_assign2_1.c

test_assign2_2.o: test_assign2_2.c
	gcc -c test_assign2_2.c

storage_mgr.o: storage_mgr.c
	gcc -c storage_mgr.c

dberror.o: dberror.c
	gcc -c dberror.c

buffer_mgr.o: buffer_mgr.c
	gcc -c buffer_mgr.c

buffer_mgr_stat.o: buffer_mgr_stat.c
	gcc -c buffer_mgr_stat.c

clean:
	rm test_assign2_1
	rm test_assign2_2
