#include "../nessemble.h"

/**
 * .enum pseudo instruction
 * @param {unsigned int} value - Enum start value
 * @param {unsigned int} inc - Enum increment
 */
void pseudo_enum(unsigned int value, unsigned int inc) {
    enum_active = TRUE;
    enum_value = value;
    enum_inc = inc;
}
