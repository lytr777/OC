#include "client.h"

client::client(int socket, uint32_t events) : events(events) {
    sockaddr_in client;
    memset(&client, 0, sizeof(sockaddr_in));
    socklen_t client_len = sizeof(sockaddr_in);
    int fd = accept(socket, (sockaddr *) &client, &client_len);

    ptr = std::make_shared<read_writer>(46, fd, events);
    //printf("client created : %d\n", fd);

    int flags;
    if ((flags = fcntl(fd, F_GETFL, 0)) == -1) perror("flag set failed");
    flags |= O_NONBLOCK;
    if (fcntl(fd, F_SETFL, flags) < 0) perror("flag set failed");
}

void client::set_pair(read_writer *pair) {
    ptr->pair = pair;
}