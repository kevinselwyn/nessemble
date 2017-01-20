#include "../nessemble.h"

/**
 * .rs pseudo instruction
 * @param {char *} label - Variable label
 * @param {int} size - Variable size
 */
void pseudo_rs(char *label, int size) {
    add_symbol(label, rsset, SYMBOL_RS);

    rsset += size;
}
