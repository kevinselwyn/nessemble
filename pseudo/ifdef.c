#include "../nessemble.h"

/**
 * .ifdef pseudo instruction
 */
void pseudo_ifdef(char *label) {
    if_depth++;
    if_active = TRUE;
    if_type = IF_IFDEF;
    if_cond = has_label(label) == TRUE ? TRUE : FALSE;
}
