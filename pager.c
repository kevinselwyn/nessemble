#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "nessemble.h"

unsigned int pager_buffer(char *buffer) {
    unsigned int rc = RETURN_OK;
    int pfds[2] = { 0, 0 };
    char *arguments[2];
    pid_t pid;

    if (pipe(pfds) < 0) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    pid = fork();

    if (pid < 0) {
        rc = RETURN_EPERM;
        goto cleanup;
    } else if (pid == 0) {
        (void)close(pfds[0]);
        (void)dup2(pfds[1], STDOUT_FILENO);
        (void)close(pfds[1]);

        printf("%s", buffer);
    } else {
        arguments[0] = "less";
        arguments[1] = NULL;

        (void)close(pfds[1]);
        (void)dup2(pfds[0], STDIN_FILENO);
        (void)close(pfds[0]);

        if (execvp(arguments[0], arguments) == 0) {
            rc = RETURN_OK;
            goto cleanup;
        }

        printf("%s", buffer);
        rc = RETURN_OK;
    }

cleanup:
    return rc;
}
