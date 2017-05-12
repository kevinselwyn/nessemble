#include "../nessemble.h"

/**
 * .ifndef pseudo instruction
 */
void pseudo_ifndef(char *label) {
    if_active = TRUE;
    if_cond[++if_depth] = has_label(label) != TRUE ? TRUE : FALSE;
}
