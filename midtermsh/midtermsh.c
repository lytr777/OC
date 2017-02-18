#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>

int ch_pid;

void handler(int s, siginfo_t * sinfo, void * v) {
    if (s == SIGINT) {
        if (ch_pid != 0)
            kill(ch_pid, SIGINT);
        else
            exit(0);
    }
}

char **split(char *buffer, char d, int length, int *size) {
    int amount = 1;
    int i;
    for (i = 0; i < length; i++) {
        if (buffer[i] == d)
            amount++;
    }
    char **sp = (char **) malloc(sizeof(char *) * amount);
    *size = amount;
    amount = 0;
    int last_d = 0;
    for (i = 0; i < length; i++) {
        if (buffer[i] == d) {
            sp[amount] = (char *) malloc(sizeof(char) * (i - last_d));
            memcpy(sp[amount], buffer + last_d, (i - last_d));
            amount++;
            last_d = i + 1;
        }
        if (i == length - 1) {
            sp[amount] = (char *) malloc(sizeof(char) * (i - last_d));
            memcpy(sp[amount], buffer + last_d, (i - last_d));
        }
    }
    return sp;
}

int exec_command(char **command) {
    int pid = fork();
    if (pid > 0) {
        ch_pid = pid;
        wait(&pid);
        ch_pid = 0;
        return 1;
    } else {
        if (execvp(command[0], command) == -1)
            printf("%s : команда не найдена\n", command[0]);
    }
    return 0;
}

int main() {
    int buff_size = 1024, offset = 0, r, cur_length = 0;
    char* buffer = (char *) malloc(sizeof(char) * buff_size);
    struct sigaction sa;
    sa.sa_sigaction = &handler;
    sigaction(SIGINT, &sa, NULL);
    while ((r = read(0, buffer + offset, buff_size - offset)) != 0) {
        cur_length = cur_length + r;
        if (buffer[cur_length - 1] != '\n') {
            int new_buff_size = 2 * buff_size;
            char *new_buffer = (char *) malloc(sizeof(char) * new_buff_size);
            memcpy(new_buffer, buffer, buff_size);
            free(buffer);
            buffer = new_buffer;
            offset = offset + buff_size;
            buff_size = new_buff_size;
        } else {
            cur_length = 0;
            int size = 0;
            //char **pipe = split(buffer, '|', offset + r, &size);
            char **sp = split(buffer, ' ', offset + r, &size);
            offset = 0;
            int flag = exec_command(sp);
            if (flag == 0)
                break;
        }
    }
    return 0;
}