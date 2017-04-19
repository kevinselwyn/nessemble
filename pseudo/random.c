#include <stdlib.h>
#include <time.h>
#include "../nessemble.h"

static unsigned long int custom_rand_next = 1;

static void custom_srand(unsigned int seed) {
    custom_rand_next = seed;
}

static int custom_rand(void) {
    custom_rand_next = custom_rand_next * 1103515245 + 12345;

    return (unsigned int)(custom_rand_next / 65536) % 32768;
}

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

    custom_srand(seed);

    for (i = 0, l = count; i < l; i++) {
        write_byte((unsigned int)custom_rand());
    }

    length_ints = 0;
}
