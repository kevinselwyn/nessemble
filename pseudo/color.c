#include "../nessemble.h"
#include "../png.h"

/**
 * .color pseudo instruction
 */
void pseudo_color() {
    unsigned int i = 0, l = 0, color = 0;
    unsigned char *rgb = NULL;

    rgb = (unsigned char *)nessemble_malloc(sizeof(unsigned char) * 3);

    for (i = 0, l = (unsigned int)length_ints; i < l; i++) {
        color = ints[i] & 0xFFFFFF;
        rgb[0] = (color >> 16) & 0xFF;
        rgb[1] = (color >> 8) & 0xFF;
        rgb[2] = color & 0xFF;

        write_byte((unsigned int)match_color(rgb));
    }

    nessemble_free(rgb);

    length_ints = 0;
}
