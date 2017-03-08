VDIR = ../
test : test.o hash.o
	gcc -o test hash.o test.o

hash.o : ../hash.c ../hash.h ../list.h ../compiler.h ../vault.h
	gcc -c $(VDIR)hash.c -o $@
test.o : hash_test.c ../vault.h hash.o
	gcc -o $@ -c hash_test.c
