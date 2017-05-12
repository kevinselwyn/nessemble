#include <stdio.h>
#include "nessemble.h"

unsigned int get_coverage() {
    unsigned int rc = RETURN_OK;
    unsigned int i = 0, j = 0, k = 0, l = 0;
    unsigned int covered = 0;

    for (i = 0, j = ines.prg; i < j; i++) {
        covered = 0;

        for (k = i * BANK_PRG, l = (i + 1) * BANK_PRG; k < l; k++) {
            if (coverage[k] == TRUE) {
                covered++;
            }
        }

        printf("PRG %02X: %5u/%-5d\n", i, covered, BANK_PRG);
    }

    for (i = 0, j = ines.chr; i < j; i++) {
        covered = 0;

        for (k = (ines.prg * BANK_PRG) + (i * BANK_CHR), l = (ines.prg * BANK_PRG) + ((i + 1) * BANK_CHR); k < l; k++) {
            if (coverage[k] == TRUE) {
                covered++;
            }
        }

        printf("CHR %02X: %5u/%-5d\n", i, covered, BANK_CHR);
    }

    return rc;
}
