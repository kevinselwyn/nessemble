#include <stdlib.h>
#include <time.h>
#include "../nessemble.h"

/**
 * .random pseudo instruction
 * @param {unsigned int} seed - Seed value
 */
void pseudo_random() {
    unsigned int i = 0, l = 0;
    unsigned int seed = 0, count = 0;

    if (length_ints < 1) {
        seed = 0;
    } else {
        seed = ints[0];
    }

    if (length_ints < 2) {
        count = 1;
    } else {
        count = ints[1];
    }

#ifndef IS_WINDOWS
    if (seed == 0) {
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        seed = (unsigned int)ts.tv_nsec;
    }
#endif /* IS_WINDOWS */

    srand(seed);

    for (i = 0, l = count; i < l; i++) {
        write_byte((unsigned int)rand());
    }

    length_ints = 0;
}
