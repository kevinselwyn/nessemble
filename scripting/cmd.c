#include <string.h>
#include "../nessemble.h"

unsigned int scripting_cmd(char *exec) {
    unsigned int rc = 0, i = 0, l = 0;
    unsigned int command_length = 0, output_length = 0;
    char buffer[1024];
    char *command = NULL;
    FILE *pipe = NULL;

    command_length = (unsigned int)strlen(exec) + (length_ints * 4);

#ifndef IS_WINDOWS
    command_length += 12;
#endif /* IS_WINDOWS */

    command = (char *)nessemble_malloc(sizeof(char) * (command_length));

    strcpy(command, exec);

    for (i = 0, l = length_ints; i < l; i++) {
        sprintf(command+strlen(command), " %u", ints[i] & 0xFF);
    }

#ifndef IS_WINDOWS
    strcat(command, " 2>/dev/null");
#endif /* IS_WINDOWS */

    if ((pipe = popen(command, "r")) == NULL) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    output_length = (unsigned int)fread(buffer, 1, 1024, pipe);

    for (i = 0, l = output_length; i < l; i++) {
        write_byte((unsigned int)buffer[i] & 0xFF);
    }

    if (pipe) {
        if ((rc = pclose(pipe)) != 0) {
            rc = RETURN_EPERM;
            goto cleanup;
        }
    }

cleanup:
    nessemble_free(command);

    return rc;
}
