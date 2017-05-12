#include "../nessemble.h"

/**
 * .endif pseudo instruction
 */
void pseudo_endif() {
    if (--if_depth == 0) {
        if_active = FALSE;
    }
}
