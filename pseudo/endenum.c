#include "../nessemble.h"

/**
 * .endenum pseudo instruction
 */
void pseudo_endenum() {
    enum_active = FALSE;
    enum_value = 0;
    enum_inc = 0;
}
