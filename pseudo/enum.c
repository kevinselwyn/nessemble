#include "../nessemble.h"

/**
 * .enum pseudo instruction
 * @param {unsigned int} value - Enum start value
 */
void pseudo_enum(unsigned int value) {
    enum_active = TRUE;
    enum_value = value;
}
