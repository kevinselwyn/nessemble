#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "nessemble.h"

unsigned int pager_buffer(char *buffer) {
    unsigned int rc = RETURN_OK;
    int pfds[2] = { 0, 0 };
    char *arguments[2];
    pid_t pid;

#ifdef IS_WINDOWS
    printf("%s", buffer);
#else /* IS_WINDOWS */
    if (pipe(pfds) < 0) {
        printf("%s", buffer);
        goto cleanup;
    }

    pid = fork();

    if (pid < 0) {
        printf("%s", buffer);
        goto cleanup;
    } else if (pid == 0) {
        UNUSED(close(pfds[0]));
        UNUSED(dup2(pfds[1], STDOUT_FILENO));
        UNUSED(close(pfds[1]));

        printf("%s", buffer);
    } else {
        arguments[0] = "less";
        arguments[1] = NULL;

        UNUSED(close(pfds[1]));
        UNUSED(dup2(pfds[0], STDIN_FILENO));
        UNUSED(close(pfds[0]));

        UNUSED(execvp(arguments[0], arguments));

        printf("%s", buffer);
        rc = RETURN_OK;
    }
#endif /* IS_WINDOWS */

cleanup:
    return rc;
}

unsigned int pager_file(char *filename) {
    unsigned int rc = RETURN_OK;
    char *content = NULL;

    if (load_file(&content, filename) == 0) {
        goto cleanup;
    }

    if ((rc = pager_buffer(content)) != RETURN_OK) {
        goto cleanup;
    }

cleanup:
    nessemble_free(content);

    return rc;
}
