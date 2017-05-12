#include "../nessemble.h"

/**
 * .else pseudo instruction
 */
void pseudo_else() {
    if_cond[if_depth] = (unsigned int)(if_cond[if_depth] == TRUE ? FALSE : TRUE);
}
