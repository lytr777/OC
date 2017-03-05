#include <string>
#include <deque>
#include <unistd.h>
#include <memory.h>

#ifndef RSHD_READWRITER_H
#define RSHD_READWRITER_H

const int BUFFER_SIZE = 512;

class read_writer {
public:
    int type;
    int fd;
    int len, offset;
    read_writer *pair;
    pid_t child;
    uint32_t events;

    read_writer(int type, int fd, uint32_t events);

    int read_fd();

    int write_fd();

private:
    char buffer[BUFFER_SIZE];
};


#endif //RSHD_READWRITER_H
