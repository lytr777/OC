#include <iostream>
#include "read_writer.h"

read_writer::read_writer(int type, int fd) : type(type), fd(fd) {
    memset(read_buffer, 0, BUFFER_SIZE);
    child = -1;
}

int read_writer::read_fd() {
    std::string str;
    ssize_t r;
    while ((r = read(fd, read_buffer, BUFFER_SIZE)) > 0)
        str += std::string(read_buffer, r);

    //std::cout << fd << " read : " << str << std::endl;
    if (r < 0 && errno != EAGAIN) {
        errno = 0;
        return -1;
    }
    //if (str.size() == 0) return 0;
    pair->write_deque.push_back(str);
    pair->write_fd();
    return str.size();
}

int read_writer::write_fd() {
    ssize_t rec = 0;
    if (write_deque.empty())
        write(fd, '\0', 0);
    while (!write_deque.empty()) {
        std::string str = write_deque.front();
        write_deque.pop_front();
        const char *buf = str.c_str();
        size_t str_length = str.size();
        size_t left = str_length;
        ssize_t recorded;

        //std::cout << fd << " write : " << str << std::endl;

        while (left > 0 && (recorded = write(fd, buf + str_length - left, left)) >= 0) {
            left -= recorded;
            rec += recorded;
        }

        if (left > 0) {
            if (errno != EAGAIN) return -1;
            write_deque.push_front(std::string(buf + str_length - left, left));
            break;
        }
    }
    return rec;
}
