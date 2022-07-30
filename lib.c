//
// Created by yuce on 16.07.2022.
//

#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <time.h>
#include <limits.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "lib.h"

#define PROG "waitport"

void print_usage(const char *argv0) {
    printf("Usage: %s [host] port [options...]\n\n"
            "Options:\n"
            "    -t: Minimum time to wait in seconds.\n"
            "        Example: 10 == 10 seconds.\n"
            "        Defaults to -1, no timeout.\n"
            "    -s: Sleep time between retries in seconds.\n"
            "        Example: 0.1 == 100 milliseconds.\n"
            "        Defaults to 1 second.\n"
            "\n", argv0);
    exit(EXIT_FAILURE);
}

void bye(const char *msg) {
    fprintf(stderr, "%s\n", msg);
    exit(EXIT_FAILURE);
}

_Bool connects(const char *host, const char *port) {
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
    _Bool ret = 0;
    // traverse the linked list to check whether one of the items is connectable.
    for (p = res; p != NULL; p = p->ai_next) {
        int s = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (s < 0) {
            continue;
        }
        int c = connect(s, p->ai_addr, p->ai_addrlen);
        if (c >= 0) {
            ret = 1;
            goto ret;
        }
    }
ret:
    freeaddrinfo(res);
    return ret;
}

static inline uint64_t time_delta_us(struct timespec a, struct timespec b) {
    return (a.tv_sec - b.tv_sec) * 1000000 + (a.tv_nsec - b.tv_nsec) / 1000;
}

void sleep_ms(long ms) {
    if (ms <= 0) {
        return;
    }
    struct timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000;
    int res;
    do {
        res = nanosleep(&ts, &ts);
    } while (res && errno == EINTR);
}

_Bool wait_until_connects(struct opts o) {
    if (o.timeout < 0) {
        o.timeout = LONG_MAX;
    }
    struct timespec tic;
    struct timespec toc;
    clock_gettime(CLOCK_MONOTONIC_RAW, &tic);
    uint64_t delta_ms;
    do {
        if (connects(o.host, o.port)) {
            return 1;
        }
        sleep_ms(o.sleep);
        clock_gettime(CLOCK_MONOTONIC_RAW, &toc);
        delta_ms = time_delta_us(toc, tic) / 1000;
    } while (delta_ms < o.timeout);
    return 0;
}

long parse_ms(char *text) {
    float timeout = strtof(text, NULL);
    return (long)(timeout * 1000.0f);
}

struct opts parse_args(int argc, char **argv) {
    struct opts r;
    memset(&r, 0, sizeof r);
    r.timeout = -1;
    r.sleep = 1000;
    int idx = 1;
    while (idx < argc) {
        char *arg = argv[idx++];
        unsigned long l = strlen(arg);
        if (l > 0 && arg[0] == '-') {
            switch (arg[1]) {
                case 't':
                    if (idx >= argc) {
                        bye("-t requires an argument");
                    }
                    r.timeout = parse_ms(argv[idx++]);
                    continue;
                case 's':
                    if (idx >= argc) {
                        bye("-s requires an argument");
                    }
                    r.sleep = parse_ms(argv[idx++]);
                    continue;
                default:
                    bye("invalid option");
            }
        }
        if (r.port == NULL) {
            r.port = arg;
            continue;
        }
        if (r.host == NULL) {
            r.host = r.port;
            r.port = arg;
            continue;
        }
        bye("host and port were alrady assigned");
    }
    if (r.port == NULL) {
        print_usage(argv[0]);
    }
    if (r.host == NULL) {
        r.host = "localhost";
    }
    if (strlen(r.port) > 5) {
        bye(PROG ": port too long");
    }
    if (strlen(r.host) > 128) {
        bye(PROG ": host too long");
    }
    return r;
}