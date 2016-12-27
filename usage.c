#include <stdio.h>
#include <string.h>
#include "nessemble.h"

static struct usage_flag usage_flags[USAGE_FLAG_COUNT] = {
    { "-o, --output <outfile.rom>", "output file" },
    { "-f, --format {NES,RAW}", "output format" },
    { "-u, --undocumented", "use undocumented opcodes" },
    { "-l, --list <listfile.txt>", "generate list of labels and constants" },
    { "-c, --check", "check syntax only" },
    { "-d, --disassemble", "disassemble infile" },
    { "-s, --simulate <infile.rom>", "start the simulator" },
    { "-r, --recipe <recipe.txt>", "recipe file for the simulator" },
    { "-h, --help", "print this message" }
};

/**
 * Program usage
 * @param {string *} exec - Executable name
 * @param {int} Return code
 */
int usage(char *exec) {
    unsigned int i = 0, l = 0;
    size_t length = 0, max_length = 0;

    printf("Usage: %s [options] <infile.asm>\n\n", exec);
    printf("Options:\n");

    for (i = 0, l = USAGE_FLAG_COUNT; i < l; i++) {
        length = strlen(usage_flags[i].invocation);

        if (length > max_length) {
            max_length = length;
        }
    }

    for (i = 0, l = USAGE_FLAG_COUNT; i < l; i++) {
        length = strlen(usage_flags[i].invocation);

        printf("  %s%*s%s\n", usage_flags[i].invocation, (int)(max_length - length + 2), " ", usage_flags[i].description);
    }

    return RETURN_USAGE;
}
