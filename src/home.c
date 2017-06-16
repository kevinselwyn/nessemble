#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
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
    char *sudo_uid = NULL;
    uid_t uid = getuid();
    struct passwd *pw;

    if (uid == 0) {
        sudo_uid = getenv("SUDO_UID");

        if (!sudo_uid) {
            rc = RETURN_EPERM;
            goto cleanup;
        }

        uid = (uid_t)atoi(sudo_uid);
    }

    pw = getpwuid(uid);

    if (!pw) {
        error_program_log(_("Could not find home directory"));

        rc = RETURN_EPERM;
        goto cleanup;
    }

    *home = (char *)nessemble_malloc(sizeof(char) * (strlen(pw->pw_dir) + 1));

    strcpy(*home, pw->pw_dir);
cleanup:
#endif /* IS_WINDOWS */

    return rc;
}

unsigned int get_home_path(char **path, unsigned int num, ...) {
    unsigned int rc = RETURN_OK, i = 0, l = 0, length = 0;
    char *arg = NULL, *home = NULL, *output = NULL;
    va_list argptr;

    if ((rc = get_home(&home)) != RETURN_OK) {
        goto cleanup;
    }

    if (!home) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    length = (unsigned int)strlen(home);

    va_start(argptr, num);

    for (i = 0, l = num; i < l; i++) {
        arg = va_arg(argptr, char *);
        length += (unsigned int)strlen(arg) + 1;
    }

    output = (char *)nessemble_malloc(sizeof(char) * (length + 1));
    sprintf(output, "%s", home);

    va_start(argptr, num);

    for (i = 0, l = num; i < l; i++) {
        arg = va_arg(argptr, char *);
        sprintf(output+strlen(output), SEP "%s", arg);
    }

    va_end(argptr);

cleanup:
    *path = output;

    nessemble_free(home);

    return rc;
}
