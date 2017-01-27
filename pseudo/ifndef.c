#include "../nessemble.h"

/**
 * .ifndef pseudo instruction
 */
void pseudo_ifndef(char *label) {
    if_depth++;
    if_active = TRUE;
    if_type = IF_IFNDEF;
    if_cond = has_label(label) != TRUE ? TRUE : FALSE;
}
