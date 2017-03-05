#include <iostream>
#include "read_writer.h"

read_writer::read_writer(int type, int fd, uint32_t events) : type(type), fd(fd), events(events) {
    memset(buffer, 0, BUFFER_SIZE);
    child = -1;
    len = 0;
    offset = 0;
}

int read_writer::read_fd() {
    std::string str;
    ssize_t r;
    r = read(fd, buffer + len, BUFFER_SIZE - len);
    //printf("%d read event : %zi\n", fd, r);
    if (r < 0 && errno != EAGAIN) {
        errno = 0;
        return -1;
    }
    len += r;
    return r;
}

int read_writer::write_fd() {
    ssize_t l = write(fd, pair->buffer + pair->offset, pair->len - pair->offset);
    if (l < 0) {
        errno = 0;
        return -1;
    }
    //printf("%d write event : %zi of %d\n", fd, l, pair->len - pair->offset);
    pair->offset += l;
    if (pair->offset == pair->len) {
        pair->len = 0;
        pair->offset = 0;
        return 1;
    } else
        return 0;
}
