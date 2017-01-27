#include "../nessemble.h"

/**
 * .else pseudo instruction
 */
void pseudo_else() {
    if_cond = (unsigned int)(if_cond == TRUE ? FALSE : TRUE);
}
