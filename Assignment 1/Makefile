all: test_assign1 test_assign1_2

test_assign1: test_assign1_1.o storage_mgr.o dberror.o
	gcc test_assign1_1.o storage_mgr.o dberror.o -o test_assign1

test_assign1_2: test_assign1_2.o storage_mgr.o dberror.o
	gcc test_assign1_2.o storage_mgr.o dberror.o -o test_assign1_2
	rm -rf *o

test_assign1_1.o: test_assign1_1.c
	gcc -c test_assign1_1.c

test_assign1_2.o: test_assign1_2.c
	gcc -c test_assign1_2.c

storage_mgr.o: storage_mgr.c
	gcc -c storage_mgr.c

dberror.o: dberror.c
	gcc -c dberror.c

clean:
	rm test_assign1 test_assign1_2
