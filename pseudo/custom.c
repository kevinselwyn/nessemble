#include <string.h>
#include "../nessemble.h"

/**
 * custom pseudo instruction
 */
void pseudo_custom(char *pseudo) {
    unsigned int i = 0, l = 0;
    size_t output_length = 0;
    char buffer[1024];
    char *exec = NULL, *command = NULL;
    FILE *pipe = NULL;

    if (pseudo_parse(&exec, pseudo) != RETURN_OK) {
        goto cleanup;
    }

    command = (char *)nessemble_malloc(sizeof(char) * (strlen(exec) + (length_ints * 4) + 1));

    strcpy(command, exec);

    for (i = 0, l = length_ints; i < l; i++) {
        sprintf(command+strlen(command), " %d", ints[i] & 0xFF);
    }

    pipe = popen(command, "r");

    if (!pipe) {
        goto cleanup;
    }

    output_length = fread(buffer, 1, 1024, pipe);

    for (i = 0, l = (unsigned int)output_length; i < l; i++) {
        write_byte(buffer[i] & 0xFF);
    }

cleanup:
    if (pipe) {
        pclose(pipe);
    }

    nessemble_free(exec);
    nessemble_free(command);

    length_ints = 0;
}
