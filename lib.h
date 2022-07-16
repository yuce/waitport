//
// Created by yuce on 16.07.2022.
//

#ifndef WAITPORT_LIB_H
#define WAITPORT_LIB_H

struct opts {
    char *host;
    char *port;
    long timeout;
    long sleep;
};

_Bool wait_until_connects(const char *host, const char *port, long timeout, long sleep);
struct opts parse_args(int argc, char **argv);

#endif //WAITPORT_LIB_H
