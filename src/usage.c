#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include "nessemble.h"
#include "static/license.h"

struct option commandline_options[USAGE_FLAG_COUNT+1] = {
    { "output",       required_argument, 0, 'o' },
    { "format",       required_argument, 0, 'f' },
    { "empty",        required_argument, 0, 'e' },
    { "undocumented", no_argument,       0, 'u' },
    { "list",         required_argument, 0, 'l' },
    { "pseudo",       required_argument, 0, 'p' },
    { "check",        no_argument,       0, 'c' },
    { "coverage",     no_argument,       0, 'C' },
    { "disassemble",  no_argument,       0, 'd' },
    { "reassemble",   no_argument,       0, 'R' },
    { "simulate",     no_argument,       0, 's' },
    { "recipe",       required_argument, 0, 'r' },
    { "version",      no_argument,       0, 'v' },
    { "license",      no_argument,       0, 'L' },
    { "help",         no_argument,       0, 'h' },
    { 0, 0, 0, 0 }
};

static struct usage_flag usage_flags[USAGE_FLAG_COUNT] = {
    { "-o, --output <outfile.rom>", "output file" },
    { "-f, --format {NES,RAW}", "output format" },
    { "-e, --empty <hex>", "empty byte value" },
    { "-u, --undocumented", "use undocumented opcodes" },
    { "-l, --list <listfile.txt>", "generate list of labels and constants" },
    { "-p, --pseudo <pseudo.txt>", "use custom pseudo-instruction functions" },
    { "-c, --check", "check syntax only" },
    { "-C, --coverage", "log data coverage" },
    { "-d, --disassemble", "disassemble infile" },
    { "-R, --reassemble", "enable reassembly" },
    { "-s, --simulate <infile.rom>", "start the simulator" },
    { "-r, --recipe <recipe.txt>", "recipe file for the simulator" },
    { "-v, --version", "display program version"},
    { "-L, --license", "display program license"},
    { "-h, --help", "print this message" }
};

static struct usage_flag usage_commands[USAGE_COMMAND_COUNT] = {
    { "init [<arg> ...]", "initialize new project" },
    { "scripts", "install scripts" },
    { "reference [<category>] [<term>]", "get reference info about assembly terms" },
    { "config [<key>] [<val>]", "list/get/set config info" },
    { "registry [<url>]", "get/set registry url" },
    { "install <package>", "install package" },
    { "uninstall <package>", "uninstall package" },
    { "publish <package.tar.gz>", "publish package"},
    { "info <package>", "get info about package" },
    { "ls", "list all installed packages" },
    { "search <term>", "search for package in registry" },
    { "adduser [<arg> ...]", "create user account" },
    { "login [<arg> ...]", "login to the registry" },
    { "logout", "logout of the registry" },
    { "forgotpassword [<arg> ...]", "send password reset email" },
    { "resetpassword [<arg> ...]", "reset password" }
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
    { ".step [X]", "Step program counter by 1 or X" },
    { ".run", "Run program" },
    { ".record XXXX <filename>", "Record writes to address XXXX" },
    { ".breakpoints", "List all breakpoints" },
    { ".add_breakpoint XXXX [<name>]", "Add breakpoint at address XXXX with optional name" },
    { ".remove_breakpoint [XXXX|<name>],...", "Remove breakpoint(s) at address XXXX or with name" },
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
unsigned int usage(char *exec) {
    int length = (int)strlen(exec);
    unsigned int rc = RETURN_USAGE;

    printf("%s: %s [%s] <infile.asm>\n", _("Usage"), exec, _("options"));
    printf("%*s<%s> [%s]\n\n", (int)(length + strlen(_("Usage")) + 3), " ", _("command"), _("args"));
    printf("%s:\n", _("Options"));

    print_usage(usage_flags, USAGE_FLAG_COUNT);

    printf("\n%s:\n", _("Commands"));

    print_usage(usage_commands, USAGE_COMMAND_COUNT);

    return rc;
}

/**
 * Simulation usage
 */
void usage_simulate() {
    printf("%s:\n\n", _("Options"));

    print_usage(simulation_usage_flags, SIMULATION_USAGE_FLAG_COUNT);

    printf("\n");
}

/**
 * Program version
 */
unsigned int version() {
    unsigned int rc = RETURN_USAGE;

    printf(PROGRAM_NAME " v" PROGRAM_VERSION "\n\n%s " PROGRAM_COPYRIGHT " " PROGRAM_AUTHOR "\n", _("Copyright"));

    return rc;
}

/**
 * Program license
 */
unsigned int license() {
    unsigned int rc = version();

    license_txt[license_txt_len-1] = '\0';

    printf("\n%s\n", (char *)license_txt);

    return rc;
}
