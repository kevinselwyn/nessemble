#include <string.h>
#include "../nessemble.h"

/**
 * .ease pseudo instruction
 * @param {char *} type - Ease type
 * @param {unsigned int} start - Ease start value
 * @param {unsigned int} steps - Ease step count
 */
void pseudo_ease(char *type, unsigned int start, unsigned int end, unsigned int steps) {
    unsigned int i = 0, j = 0, k = 0, l = 0;
    unsigned int found = FALSE;
    float value = 0.0;
    size_t length = 0;

    length = strlen(type);
    type[length - 1] = '\0';

    if (end > 0xFF) {
        end = 0xFF;
    }

    for (i = 0, j = EASING_COUNT; i < j; i++) {
        if (strcmp(type+1, easings[i].type) == 0) {
            for (k = 0, l = steps; k < l; k++) {
                value = (*easings[i].func)((float)k / ((float)l - 1), 0, 1, 1) * (end - start);
                write_byte(((unsigned int)value + start) & 0xFF);
            }

            found = TRUE;
        }
    }

    if (found != TRUE) {
        error("Invalid easing type `%s`", type+1);
    }
}
