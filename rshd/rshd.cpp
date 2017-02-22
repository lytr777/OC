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
    ev.events = events;
    ev.data.ptr = (void *) ptr;
    //printf("event add : %d\n", ptr->fd);
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, ptr->fd, &ev) == -1) {
        perror("epoll_ctl_add");
    }
}

void change_event(int epfd, read_writer *ptr, uint32_t events) {
    epoll_event ev;
    ev.events = events;
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

//    read_writer *disc_pair[2];
//    disc_pair[event_wrapper->type - 46] = event_wrapper;
//    disc_pair[event_wrapper->pair->type - 46] = event_wrapper->pair;
//    printf("disconnect : %d\n", disc_pair[0]->fd);
//    for (auto it = clients.begin(); it != clients.end(); ++it) {
//        if (it->ptr.get() == disc_pair[0]) {
//            epoll_ctl(epfd, EPOLL_CTL_DEL, disc_pair[0]->fd, NULL);
//            clients.erase(it);
//            close(disc_pair[0]->fd);
//            printf("close : %d\n", disc_pair[0]->fd);
//            break;
//        }
//    }
//    for (auto it = terminals.begin(); it != terminals.end(); ++it) {
//        if (it->ptr.get() == disc_pair[1]) {
//            epoll_ctl(epfd, EPOLL_CTL_DEL, disc_pair[1]->fd, NULL);
//            terminals.erase(it);
//            close(disc_pair[1]->fd);
//            printf("close : %d\n", disc_pair[1]->fd);
//            break;
//        }
//    }
    if (kill)
        printf("kill disconnect pid : %d is %d\n", event_wrapper->child,
               wait_kill(event_wrapper->child));
}

int main(int argc, char **argv) {

    make_daemon();

    if (argc < 2) {
        perror("port_not_specified");
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

                client *clt = new client(srv->fd);
                terminal *trm = new terminal();

                clt->set_pair(trm->ptr.get());
                trm->set_pair(clt->ptr.get());

                clients.push_back(*clt);
                terminals.push_back(*trm);

                add_event(epfd, clt->ptr.get(), EPOLLIN | EPOLLOUT | EPOLLRDHUP);
                add_event(epfd, trm->ptr.get(), EPOLLIN | EPOLLOUT);

                trm->open_pty(epfd, srv->ptr, clt->ptr);
            } else {
                int res = -1;
                if (events[i].events & EPOLLRDHUP) {
                    //kill(event_wrapper->child, VEOF);
                    //res = event_wrapper->read_fd();
                    res = event_wrapper->pair->write_fd();
                    //printf("zero read %d : %d\n", res, event_wrapper->fd);
                    disconnect(epfd, event_wrapper, 1);
                } else {
                    if ((events[i].events & EPOLLIN) != 0) {
                        res = event_wrapper->read_fd();
                        if (res > 0) {
//                            change_event(epfd, event_wrapper->pair, 0);
//                            change_event(epfd, event_wrapper, EPOLLIN);
                            //printf("%d read event : %d\n", event_wrapper->fd, res);
                        }
                        //printf("%d read event : %d\n", event_wrapper->fd, res);
                    } else if ((events[i].events & EPOLLOUT) != 0) {
                        res = event_wrapper->write_fd();
                        if (res > 0) {
//                            change_event(epfd, event_wrapper, EPOLLIN);
//                            change_event(epfd, event_wrapper->pair, 0);
                            //printf("%d write event : %d\n", event_wrapper->fd, res);
                        }
                    }
                    if (res == -1)
                        disconnect(epfd, event_wrapper, 1);
                }
                if (!check_pid(event_wrapper->child)) {
                    disconnect(epfd, event_wrapper, 0);
                    //printf("check_pid : 0\n");
                }
            }
        }
    }

//    for (auto it = clients.begin(); it != clients.end(); ++it) {
//        wait_kill(it->ptr->child);
//    }
    return 0;
}
