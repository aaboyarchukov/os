#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/utsname.h>

int main(void) {
    struct utsname u;
    if (uname(&u) == -1) {
        perror("uname");
        return EXIT_FAILURE;
    }

    printf("kernel : %s %s (%s)\n", u.sysname, u.release, u.machine);
    printf("pid    : %d\n", getpid());
    printf("page   : %ld bytes\n", sysconf(_SC_PAGESIZE));
    printf("cpus   : %ld\n", sysconf(_SC_NPROCESSORS_ONLN));

    /* Linux-only: this does not exist on macOS, which is the whole point of
       building inside the container. */
    int ep = epoll_create1(0);
    if (ep == -1) {
        perror("epoll_create1");
        return EXIT_FAILURE;
    }
    printf("epoll  : fd %d\n", ep);
    close(ep);

    return EXIT_SUCCESS;
}
