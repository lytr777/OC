CC=gcc
CFLAGS = -c -Wall

midtermsh : midtermsh.o
	$(CC) midtermsh.o -o midtermsh

midtersh.o: midtersh.c
	$(CC) $(CFLAGS) midtermsh.c

clean:
	rm -f *o midtermsh