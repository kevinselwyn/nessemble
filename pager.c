#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "nessemble.h"

unsigned int pager_buffer(char *buffer) {
    unsigned int rc = RETURN_OK;
    int pfds[2] = { 0, 0 };
    char *arguments[2];
    pid_t pid;

#ifdef WIN32
    printf("%s", buffer);
#else /* WIN32 */
    if (pipe(pfds) < 0) {
        printf("%s", buffer);
        goto cleanup;
    }

    pid = fork();

    if (pid < 0) {
        printf("%s", buffer);
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

        (void)execvp(arguments[0], arguments);

        printf("%s", buffer);
        rc = RETURN_OK;
    }
#endif /* WIN32 */

cleanup:
    return rc;
}
