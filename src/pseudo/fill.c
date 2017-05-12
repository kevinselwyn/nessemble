#include "../nessemble.h"

/**
 * .fill pseudo instruction
 */
void pseudo_fill() {
    unsigned int i = 0, l = 0;
    unsigned int count = 0, value = 0;

    if (length_ints < 1) {
        yyerror(_("Not enough .fill arguments"));
        goto cleanup;
    }

    count = ints[0];

    if (length_ints < 2) {
        value = 0xFF;
    } else {
        value = ints[1];
    }

    for (i = 0, l = count; i < l; i++) {
        write_byte(value);
    }

cleanup:
    length_ints = 0;
}
