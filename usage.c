#include <stdio.h>
#include "nessemble.h"

/**
 * Program usage
 * @param {string *} exec - Executable name
 * @param {int} Return code
 */
int usage(char *exec) {
    printf("Usage: %s [options] <infile.asm>\n\n", exec);
    printf("Options:\n");
    printf("  -o, --output <outfile.rom>   output file\n");
    printf("  -d, --disassemble            disassemble infile\n");
    printf("  -f, --format (NES|RAW)       output format\n");
    printf("  -u, --undocumented           use undocumented opcodes\n");
    printf("  -h, --help                   print this message\n");

    return RETURN_USAGE;
}
