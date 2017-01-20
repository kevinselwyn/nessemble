#include <string.h>
#include "../nessemble.h"

/**
 * .segment pseudo instruction
 * @param {char *} string - Segment name
 */
void pseudo_segment(char *string) {
    size_t length = 0;

    length = strlen(string);

    strcpy(segment, string+1);
    segment[length-2] = '\0';

    if (length-2 > 3 && strncmp(segment, "PRG", 3) == 0) {
        pseudo_prg((unsigned int)dec2int(segment+3));
    }

    if (length-2 > 3 && strncmp(segment, "CHR", 3) == 0) {
        pseudo_chr((unsigned int)dec2int(segment+3));
    }
}
