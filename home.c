#include <string.h>
#include <unistd.h>
#include "nessemble.h"

#ifdef IS_WINDOWS
#include <windows.h>
#else /* IS_WINDOWS */
#include <pwd.h>
#endif /* IS_WINDOWS */

unsigned int get_home(char **home) {
    unsigned int rc = RETURN_OK;

#ifdef IS_WINDOWS
    char *path = (char *)nessemble_malloc(sizeof(char) * 1024);

    sprintf(path, "%s%s", getenv("HOMEDRIVE"), getenv("HOMEPATH"));

    *home = path;
#else /* IS_WINDOWS */
    struct passwd *pw = getpwuid(getuid());

    if (!pw) {
        error_program_log("Could not find home directory");

        rc = RETURN_EPERM;
        goto cleanup;
    }

    *home = (char *)nessemble_malloc(sizeof(char) * (strlen(pw->pw_dir) + 1));

    strcpy(*home, pw->pw_dir);
#endif /* IS_WINDOWS */

cleanup:
    return rc;
}
