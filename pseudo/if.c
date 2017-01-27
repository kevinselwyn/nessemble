#include "../nessemble.h"

/**
 * .if pseudo instruction
 */
void pseudo_if(unsigned int cond) {
    if_active = TRUE;
    if_cond[++if_depth] = cond != 0 ? TRUE : FALSE;
}
