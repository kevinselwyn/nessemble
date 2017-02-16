#include <stdio.h>
#include <string.h>
#include "nessemble.h"
#include "license.h"

static struct usage_flag usage_flags[USAGE_FLAG_COUNT] = {
    { "-o, --output <outfile.rom>", "output file" },
    { "-f, --format {NES,RAW}", "output format" },
    { "-u, --undocumented", "use undocumented opcodes" },
    { "-l, --list <listfile.txt>", "generate list of labels and constants" },
    { "-c, --check", "check syntax only" },
    { "-d, --disassemble", "disassemble infile" },
    { "-s, --simulate <infile.rom>", "start the simulator" },
    { "-r, --recipe <recipe.txt>", "recipe file for the simulator" },
    { "-v, --version", "display program version"},
    { "-L, --license", "display program license"},
    { "-h, --help", "print this message" }
};

static struct usage_flag usage_commands[USAGE_COMMAND_COUNT] = {
    { "init", "initialize new project" },
    { "reference [<category>] [<term>]", "get reference info about assembly terms" },
    { "registry [<url>]", "get/set registry url" },
    { "install <package>", "install package" },
    { "uninstall <package>", "uninstall package" },
    { "info <package>", "get info about package" },
    { "ls", "list all packages in registry" },
    { "search <term>", "search for package in registry" }
};

static struct usage_flag simulation_usage_flags[SIMULATION_USAGE_FLAG_COUNT] = {
    { ".registers [register=XXXX,...]", "Print registers (sets registers w/ options)" },
    { ".flags [flag=X,...]", "Print flags (sets flags w/ options)" },
    { ".fill XXXX NN ...", "Fill memory address with NN byte(s)" },
    { ".disassemble XXXX:XXXX", "Disassemble instructions between addresses" },
    { ".instruction", "Print next instruction" },
    { ".memory XXXX[:XXXX]", "Print memory in address range" },
    { ".cycles", "Print count of CPU cycles" },
    { ".goto XXXX", "Set program counter to XXXX" },
    { ".step X", "Step program counter by 1 or X" },
    { ".run", "Run program" },
    { ".record XXXX <filename>", "Record writes to address XXXX" },
    { ".breakpoints", "List all breakpoints" },
    { ".add_breakpoint XXXX <name>", "Add breakpoint at address XXXX with optional name" },
    { ".remove_breakpoint <name>", "Remove breakpoint(s) at address XXXX or with name" },
    { ".quit", "Quit" },
    { ".help", "Print this message" }
};

/**
 * Usage printer
 * @param {struct usage_flag *} usage_flags - Usage flag struct
 * @param {unsigned int} size - Flag count
 */
static void print_usage(struct usage_flag *usage_flags, unsigned int size) {
    unsigned int i = 0, l = 0;
    size_t length = 0, max_length = 0;

    for (i = 0, l = size; i < l; i++) {
        length = strlen(usage_flags[i].invocation);

        if (length > max_length) {
            max_length = length;
        }
    }

    for (i = 0, l = size; i < l; i++) {
        length = strlen(usage_flags[i].invocation);

        printf("  %s%*s%s\n", usage_flags[i].invocation, (int)(max_length - length + 2), " ", usage_flags[i].description);
    }
}

/**
 * Program usage
 * @param {string *} exec - Executable name
 * @param {int} Return code
 */
int usage(char *exec) {
    int length = (int)strlen(exec);

    printf("Usage: %s [options] <infile.asm>\n", exec);
    printf("%*s<command> [args]\n\n", length + 8, " ");
    printf("Options:\n");

    print_usage(usage_flags, USAGE_FLAG_COUNT);

    printf("\nCommands:\n");

    print_usage(usage_commands, USAGE_COMMAND_COUNT);

    return RETURN_USAGE;
}

/**
 * Simulation usage
 */
void usage_simulate() {
    printf("Options:\n\n");

    print_usage(simulation_usage_flags, SIMULATION_USAGE_FLAG_COUNT);

    printf("\n");
}

/**
 * Program version
 */
int version() {
    printf(PROGRAM_NAME " v" PROGRAM_VERSION "\n\nCopyright " PROGRAM_COPYRIGHT " " PROGRAM_AUTHOR "\n");

    return RETURN_USAGE;
}

/**
 * Program license
 */
int license() {
    int rc = version();

    license_txt[license_txt_len-1] = '\0';

    printf("\n%s\n", (char *)license_txt);

    return rc;
}
