#include <unistd.h>

int main () {
char buffer[1024];
int i;
while ((i = read(0 , buffer, 1024)) > 0)
	write(1, buffer, i);
return 0;
}

