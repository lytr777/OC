all : cat
clean :
	rm -f cat.o cat
cat.o : cat.c
	gcc -c cat.c -o cat.o
cat : cat.o
	gcc -s cat.o -o cat
