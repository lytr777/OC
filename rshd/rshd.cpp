#include <fstream>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "read_writer.h"
#include "client.h"
#include "terminal.h"
#include "server.h"

const int EPOLL_SIZE = 10;

void make_daemon() {
    auto child = fork();
    if (child == -1) perror("fork fail");
    if (child != 0) {
        exit(0);
    } else {
        setsid();
        child = fork();
        if (child == 0) {
            return;
        } else {
            printf("pid: %d\n", child);
            std::ofstream ouf("/tmp/rshd.pid");
            ouf << child;
            ouf.close();
            exit(0);
        }
    }
}

int wait_kill(pid_t proc) {
    int status;
    kill(proc, SIGKILL);
    waitpid(proc, &status, 0);
    return status;
}

int check_pid(pid_t proc) {
    //printf("waitpid : %d\n", waitpid(proc, 0, WNOHANG));
    while (waitpid(proc, 0, WNOHANG) > 0) {}

    if (kill(proc, 0) == 0)
        return 1;
    return 0;
}

void add_event(int epfd, read_writer *ptr, uint32_t events) {
    epoll_event ev;
    ev.events = 0;
    ev.events |= events;
    ev.data.ptr = (void *) ptr;
    //printf("event add : %d\n", ptr->fd);
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, ptr->fd, &ev) == -1) {
        perror("epoll_ctl_add");
    }
}

void change_event(int epfd, read_writer *ptr, uint32_t &events) {
    epoll_event ev;
    ev.events = 0;
    ev.events |= events;
    ev.data.ptr = (void *) ptr;
    //printf("event add : %d\n", ptr->fd);
    if (epoll_ctl(epfd, EPOLL_CTL_MOD, ptr->fd, &ev) == -1) {
        perror("epoll_ctl_mod");
    }
}

std::vector<client> clients;
std::vector<terminal> terminals;

void disconnect(int epfd, read_writer *event_wrapper, bool kill) {
    epoll_ctl(epfd, EPOLL_CTL_DEL, event_wrapper->fd, NULL);
    close(event_wrapper->fd);
    //printf("close : %d\n", event_wrapper->fd);

    epoll_ctl(epfd, EPOLL_CTL_DEL, event_wrapper->pair->fd, NULL);
    close(event_wrapper->pair->fd);
    //printf("close : %d\n", event_wrapper->pair->fd);

    if (kill && event_wrapper->child != -1)
        //printf("kill disconnect pid : %d is %d\n", event_wrapper->child,
               wait_kill(event_wrapper->child);//);
}

int main(int argc, char **argv) {

    make_daemon();

    if (argc < 2) {
        perror("port_not_specified\n");
        return -1;
    }
    int port = atoi(argv[1]);

    server *srv = new server(port);
    if (srv->start() == -1)
        return -1;

    int events_count;
    int epfd = epoll_create(EPOLL_SIZE);
    if (epfd == -1) perror("epoll_create");
    //printf("epoll created : %d\n", epfd);

    epoll_event events[EPOLL_SIZE];
    add_event(epfd, srv->ptr.get(), EPOLLIN | EPOLLET | EPOLLPRI);
    while (1) {
        events_count = epoll_wait(epfd, events, EPOLL_SIZE, -1);
        if (events_count < 1) break;
        //printf("events : %d\n", events_count);

        for (int i = 0; i < events_count; i++) {
            read_writer *event_wrapper = (read_writer *) events[i].data.ptr;
            if (event_wrapper->type == 45) {
                //printf("new connect : %d\n", event_wrapper->fd);

                client *clt = new client(srv->fd, EPOLLIN);
                terminal *trm = new terminal(EPOLLIN);

                clt->set_pair(trm->ptr.get());
                trm->set_pair(clt->ptr.get());

                clients.push_back(*clt);
                terminals.push_back(*trm);

                add_event(epfd, clt->ptr.get(), clt->events);
                add_event(epfd, trm->ptr.get(), trm->events);

                trm->open_pty(epfd, srv->ptr, clt->ptr);
            } else {
                int res;
                if (events[i].events & EPOLLHUP) {
                    disconnect(epfd, event_wrapper, 1);
                } else if (events[i].events & EPOLLIN) {
                    res = event_wrapper->read_fd();
                    if (res > 0) {
                        event_wrapper->pair->events |= EPOLLOUT;
                        change_event(epfd, event_wrapper->pair, event_wrapper->pair->events);
                        event_wrapper->events &= ~EPOLLIN;
                        change_event(epfd, event_wrapper, event_wrapper->events);
                    } else
                        disconnect(epfd, event_wrapper, 1);
                } else if (events[i].events & EPOLLOUT) {
                    res = event_wrapper->write_fd();
                    if (res > 0) {
                        event_wrapper->pair->events |= EPOLLIN;
                        change_event(epfd, event_wrapper->pair, event_wrapper->pair->events);
                        event_wrapper->events &= ~EPOLLOUT;
                        change_event(epfd, event_wrapper, event_wrapper->events);
                    }
                }

                if (!check_pid(event_wrapper->child)) {
                    disconnect(epfd, event_wrapper, 0);
                    //printf("check_pid : %d\n", event_wrapper->child);
                }
            }
        }
    }

    return 0;
}
