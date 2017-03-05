#include "server.h"

server::server(int port) : port(port) {
    memset(&srv, 0, sizeof(srv));

    srv.sin_family = AF_INET;
    srv.sin_port = htons(port);
    srv.sin_addr.s_addr = INADDR_ANY;

    fd = socket(AF_INET, SOCK_STREAM, 0);
}

int server::start() {
    if (bind(fd, (sockaddr *) &srv, sizeof(srv)) == -1) {
        perror("Can't bind");
        return -1;
    }

    if (listen(fd, MAX_CONNECTIONS) == -1) {
        perror("Can't listen");
        return -1;
    }

    //printf("Waiting on port: %d\n", port);

    ptr = std::make_shared<read_writer>(45, fd, 0);
    //printf("server created : %d\n", ptr->fd);
    return 0;
}