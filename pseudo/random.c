#include <stdlib.h>
#include <time.h>
#include "../nessemble.h"

/**
 * .random pseudo instruction
 * @param {unsigned int} seed - Seed value
 */
void pseudo_random(unsigned int seed, unsigned int count) {
    unsigned int i = 0, l = 0;

    if (seed == 0) {
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        seed = (unsigned int)ts.tv_nsec;
    }

    srand(seed);

    for (i = 0, l = count; i < l; i++) {
        write_byte((unsigned int)rand());
    }
}
