
#include "lib.h"

int main(int argc, char **argv) {
    struct opts o = parse_args(argc, argv);
    if (wait_until_connects(o)) {
        return 0;
    }
    return 1;
}
