#include "../nessemble.h"

/**
 * .ifdef pseudo instruction
 */
void pseudo_ifdef(char *label) {
    if_active = TRUE;
    if_cond[++if_depth] = has_label(label) == TRUE ? TRUE : FALSE;
}
