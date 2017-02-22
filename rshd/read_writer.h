#include <string>
#include <deque>
#include <unistd.h>
#include <memory.h>

#ifndef RSHD_READWRITER_H
#define RSHD_READWRITER_H

const int BUFFER_SIZE = 1024;

class read_writer {
public:
    int type;
    int fd;
    read_writer *pair;
    pid_t child;

    read_writer(int type, int fd);

    int read_fd();

    int write_fd();

private:
    char read_buffer[BUFFER_SIZE];
    std::deque<std::string> write_deque;
};


#endif //RSHD_READWRITER_H
