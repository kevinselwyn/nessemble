#include "../nessemble.h"

/**
 * .endif pseudo instruction
 */
void pseudo_endif() {
    if_depth--;

    if (if_depth == 0) {
        if_active = FALSE;
    }
}
