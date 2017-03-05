#include <memory>
#include <fcntl.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <vector>
#include <sys/epoll.h>
#include "read_writer.h"
#include "client.h"

#ifndef RSHD_TERMINAL_H
#define RSHD_TERMINAL_H


class terminal {
public:
    std::shared_ptr<read_writer> ptr;
    uint32_t events;

    terminal(uint32_t events);

    void set_pair(read_writer *pair);

    void open_pty(int epfd, std::shared_ptr<read_writer> server_ptr, std::shared_ptr<read_writer> client_ptr);
};


#endif //RSHD_TERMINAL_H
