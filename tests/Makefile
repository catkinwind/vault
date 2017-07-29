SRCDIR = ../src

HASHES = hash.c

HASH_SRC = $(addprefix $(SRCDIR)/, $(HASHES))

test : test.o hash.o
	gcc -o test hash.o test.o

hash.o : $(HASH_SRC)
	gcc -c $(HASH_SRC) -o $@
test.o : hash_test.c hash.o
	gcc -o $@ -c hash_test.c
