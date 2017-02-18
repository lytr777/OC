CFLAGS = -c -Wall

sigusr : sigusr.o
	gcc -s sigusr.o -o sigusr

sigusr.o : sigusr.c
	gcc $(CFLAGS) sigusr.c

clean :
	rm -f *o sigusr