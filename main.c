#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>


#define PROG "waitport"

void print_usage(const char *argv0) {
    printf("Usage: %s host port", argv0);
    exit(EXIT_FAILURE);
}

void bye(const char *msg) {
    fputs(msg, stderr);
    exit(EXIT_FAILURE);
}

bool connects(const char *host, const char *port) {
    struct addrinfo hints;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    int status = 0;
    struct addrinfo *res;
    if ((status = getaddrinfo(host, port, &hints, &res)) != 0) {
        bye(gai_strerror(status));
    }
    struct addrinfo *p = NULL;
    // traverse the linked list to check whether one of the items is connectable.
    for (p = res; p != NULL; p = p->ai_next) {
        int s = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (s < 0) {
            continue;
        }
        int c = connect(s, p->ai_addr, p->ai_addrlen);
        freeaddrinfo(res);
        if (c >= 0) {
            return true;
        }
    }
    return false;
}

int main(int argc, char **argv) {
    if (argc != 3) {
        print_usage(argv[0]);
    }
    const char *host = argv[1];
    if (strlen(host) > 128) {
        bye(PROG ": host too long");
    }
    const char *port = argv[2];
    if (strlen(port) > 5) {
        bye(PROG ": port too long");
    }
    while (!connects(host, port)) {
        sleep(1);
    }
    return 0;
}
