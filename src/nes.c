#include <stdio.h>
#include "nessemble.h"

int nes_header(FILE *outfile) {
    int i = 0, l = 0, byte = 0;

    if (is_flag_nes2_0() == TRUE) {
        return nes2_0_header(outfile);
    }

    UNUSED(fwrite("NES", 3, 1, outfile)); /* 0-2 */

    byte = 0x1A;
    UNUSED(fwrite(&byte, 1, 1, outfile)); /* 3 */

    UNUSED(fwrite(&ines.prg, 1, 1, outfile)); /* 4 */
    UNUSED(fwrite(&ines.chr, 1, 1, outfile)); /* 5 */

    byte = 0;
    byte |= ines.mir & 0x01;
    byte |= (ines.trn & 0x01) << 0x02;
    byte |= (ines.map & 0x0F) << 0x04;

    UNUSED(fwrite(&byte, 1, 1, outfile)); /* 6 */

    byte = 0;
    byte |= (ines.map & 0xF0);

    UNUSED(fwrite(&byte, 1, 1, outfile)); /* 7 */

    byte = 0;

    UNUSED(fwrite(&byte, 1, 1, outfile)); /* 8 */
    UNUSED(fwrite(&byte, 1, 1, outfile)); /* 9 */
    UNUSED(fwrite(&byte, 1, 1, outfile)); /* 10 */

    for (i = 11, l = 16; i < l; i++) {
        UNUSED(fwrite(&byte, 1, 1, outfile)); /* 11-15 */
    }

    return 0;
}

int nes2_0_header(FILE *outfile) {
    

    return 0;
}
