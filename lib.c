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
    fprintf(stderr, 
            "Usage: %s -p port [options...]\n"
            "       %s -p port -h host [options...]\n"
            "       %s -p port -t seconds -s seconds\n\n"
            "Options:\n"
            "    -p: The port number.\n"
            "        Example: 9701\n"
            "        There is no default value, it is must have argument.\n"
            "    -h: Name of the host.\n"
            "        Example: foobar.com.\n"
            "        Defaults to localhost.\n"
            "    -t: Minimum time to wait in seconds.\n"
            "        Example: 10 == 10 seconds.\n"
            "        Defaults to -1, no timeout.\n"
            "    -s: Sleep time between retries in seconds.\n"
            "        Example: 0.1 == 100 milliseconds.\n"
            "        Defaults to 1 second.\n"
            "\n", argv0, argv0, argv0);
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
    clock_gettime(_CLOCK_MONOTONIC, &tic);
    uint64_t delta_ms;
    do {
        if (connects(o.host, o.port)) {
            return 1;
        }
        sleep_ms(o.sleep);
        clock_gettime(_CLOCK_MONOTONIC, &toc);
        delta_ms = time_delta_us(toc, tic) / 1000;
    } while (delta_ms < o.timeout);
    return 0;
}

long parse_ms(char *text) {
    float timeout = strtof(text, NULL);
    return (long)(timeout * 1000.0f);
}

struct opts parse_args(int argc, char **argv) {
    int opt;
    _Bool pfnd = 0;
    _Bool sfnd = 0; 
    _Bool tfnd = 0;
    _Bool hfnd = 0;
    struct opts r = {.host = NULL, .port = NULL, .timeout = -1, .sleep = 1000};

    while ((opt = getopt(argc, argv, "p:s:t:h:")) != -1) {
        switch (opt) {
            case 'p':
                if (strlen(optarg) > 5) {
                    bye(PROG ": port too long");
                }
                pfnd = 1;
                r.port = optarg;
                break;

            case 's':
                sfnd = 1;
                r.sleep = parse_ms(optarg);
                break;

            case 't':
                tfnd = 1;
                r.timeout = parse_ms(optarg);
                break;

            case 'h':
                if (strlen(optarg) > 128) {
                    bye(PROG ": host too long");
                }
                hfnd = 1;
                r.host = optarg;
                break;

            default:
                /* precise message given by getopt */
                break;
        }
    }

    if (!hfnd) {
        r.host = "localhost";
    }

    if (!pfnd || (optind < argc)) {
        print_usage(*argv);
    }
    
    return r;
}
