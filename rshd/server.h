#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string>
#include <deque>
#include <memory.h>
#include <memory>
#include "read_writer.h"

#ifndef RSHD_SERVER_H
#define RSHD_SERVER_H

const size_t MAX_CONNECTIONS = 10;

class server {
public:
    std::shared_ptr<read_writer> ptr;
    struct sockaddr_in srv;
    int fd, port;

    server(int port);

    int start();
};


#endif //RSHD_SERVER_H
