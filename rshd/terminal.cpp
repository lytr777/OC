#include "terminal.h"

terminal::terminal(uint32_t events) : events(events) {
    int fd = posix_openpt(O_RDWR);
    if (fd < 0) perror("Open pty err");
    if (grantpt(fd) || unlockpt(fd)) perror("Unlocking pty err");

    ptr = std::make_shared<read_writer>(47, fd, events);
    //printf("pty created : %d\n", fd);

    int flags;
    if ((flags = fcntl(fd, F_GETFL, 0)) == -1) perror("flag set failed");
    flags |= O_NONBLOCK;
    if (fcntl(fd, F_SETFL, flags) < 0) perror("flag set failed");
}

void terminal::set_pair(read_writer *pair) {
    ptr->pair = pair;
}

void terminal::open_pty(int epfd, std::shared_ptr<read_writer> server_ptr, std::shared_ptr<read_writer> client_ptr) {
    int slave = open(ptsname(ptr->fd), O_RDWR);

    //printf("slave open : %d\n", slave);
    int ch = fork();
    if (ch == 0) {
        client_ptr.reset();
        ptr.reset();
        server_ptr.reset();
        close(epfd);

        struct termios term_settings;
        tcgetattr(slave, &term_settings);
        term_settings.c_lflag &= ~(ECHO | ECHONL | ICANON);
        tcsetattr(slave, TCSANOW, &term_settings);

        dup2(slave, STDIN_FILENO);
        dup2(slave, STDOUT_FILENO);
        dup2(slave, STDERR_FILENO);
        close(slave);

        setsid();
        ioctl(0, TIOCSCTTY, 1);

        execlp("/bin/sh", "sh", NULL);
    } else {
        close(slave);
        ptr->pair->child = ch;
        ptr->child = ch;
    }
}