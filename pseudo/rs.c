#include "../nessemble.h"

/**
 * .rs pseudo instruction
 * @param {char *} label - Variable label
 * @param {int} size - Variable size
 */
void pseudo_rs(char *label, int size) {
    if (enum_active == TRUE) {
        add_symbol(label, enum_value, SYMBOL_ENUM);

        enum_value += size;
    } else {
        add_symbol(label, rsset, SYMBOL_RS);

        rsset += size;
    }
}
