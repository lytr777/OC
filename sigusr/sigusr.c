#include <stdio.h>
#include <unistd.h>
#include <signal.h>

void handler(int s, siginfo_t * sinfo, void * v) {
	if (s == SIGUSR1)
		printf("SIGUSR1");
	else if (s == SIGUSR2)
		printf("SIGUSR2");
	printf(" from %d\n", sinfo->si_pid);
}

int main() {
	struct sigaction sa;
	
	sa.sa_sigaction = &handler;
	sigaction(SIGUSR1, &sa, NULL);	
	sigaction(SIGUSR2, &sa, NULL);	

	if (sleep(10) == 0) printf("No signals were caught\n");
	return 0;
}
