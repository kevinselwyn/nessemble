#include <string.h>
#include "../nessemble.h"

/**
 * .ease pseudo instruction
 * @param {char *} type - Ease type
 */
void pseudo_ease(char *type) {
    unsigned int i = 0, j = 0, k = 0, l = 0;
    unsigned int start = 0, end = 0x10, steps = 0x10;
    unsigned int found = FALSE;
    float value = 0.0;
    size_t length = 0;

    length = strlen(type);
    type[length - 1] = '\0';

    if (length_ints >= 1) {
        start = ints[0];
    }

    if (length_ints >= 2) {
        end = ints[1];
    }

    if (length_ints >= 3) {
        steps = ints[2];
    }

    if (end > 0xFF) {
        end = 0xFF;
    }

    for (i = 0, j = EASING_COUNT; i < j; i++) {
        if (strcmp(type+1, easings[i].type) == 0) {
            for (k = 0, l = steps; k < l; k++) {
                if (end > start) {
                    value = (*easings[i].func)((float)k / ((float)l - 1), 0, 1, 1) * (end - start);
                    write_byte(((unsigned int)value + start) & 0xFF);
                } else {
                    value = (*easings[i].func)((float)(l - k - 1) / ((float)l - 1), 0, 1, 1) * (start - end);
                    write_byte(((unsigned int)value + end) & 0xFF);
                }
            }

            found = TRUE;
        }
    }

    if (found != TRUE) {
        error("Invalid easing type `%s`", type+1);
    }

    length_ints = 0;
}
