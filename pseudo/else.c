#include "../nessemble.h"

/**
 * .else pseudo instruction
 */
void pseudo_else() {
    switch (if_type) {
    case IF_IF:
        if_cond = if_cond == TRUE ? FALSE : TRUE;
        break;
    case IF_IFDEF:
        if_type = IF_IFNDEF;
        break;
    case IF_IFNDEF:
        if_type = IF_IFDEF;
        break;
    }
}
