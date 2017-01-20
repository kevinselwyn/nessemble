#include "../nessemble.h"

/**
 * .else pseudo instruction
 */
void pseudo_else() {
    if (if_type == IF_IFDEF) {
        if_type = IF_IFNDEF;
    } else {
        if_type = IF_IFDEF;
    }
}
