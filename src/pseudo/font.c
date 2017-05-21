#include "../nessemble.h"
#include "../font.h"

/**
 * .font pseudo instruction
 */
void pseudo_font() {
    unsigned int i = 0, j = 0, k = 0, l = 0;
    unsigned int ascii_start = 0x00, ascii_end = 0x00;

    if (length_ints < 1) {
        yyerror(_("Not enough .font arguments"));
        goto cleanup;
    }

    ascii_start = ints[0];

    if (length_ints < 2) {
        ascii_end = 0x65;
    } else {
        ascii_end = ints[1];
    }

    if (ascii_start < ascii_end) {
        for (i = ascii_start, j = ascii_end; i <= j; i++) {
            for (k = 0, l = 0x10; k < l; k++) {
                write_byte(font[(i * 0x10) + k]);
            }
        }
    } else if (ascii_start > ascii_end) {
        for (i = ascii_end, j = ascii_start; i <= j; i++) {
            for (k = 0, l = 0x10; k < l; k++) {
                write_byte(font[(i * 0x10) + k]);
            }
        }
    } else {
        for (k = 0, l = 0x10; k < l; k++) {
            write_byte(font[(ascii_start * 0x10) + k]);
        }
    }

cleanup:
    length_ints = 0;
}
