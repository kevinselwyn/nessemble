#include "../nessemble.h"

/**
 * .if pseudo instruction
 */
void pseudo_if(unsigned int cond) {
    if_depth++;
    if_active = TRUE;
    if_type = IF_IF;
    if_cond = cond != 0 ? TRUE : FALSE;
}
