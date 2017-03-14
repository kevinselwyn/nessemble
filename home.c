#include <string.h>
#include <unistd.h>
#include "nessemble.h"

#ifdef IS_WINDOWS
#include <windows.h>
#include <shlobj.h>
#else /* IS_WINDOWS */
#include <pwd.h>
#endif /* IS_WINDOWS */

unsigned int get_home(char **home) {
    unsigned int rc = RETURN_OK;

#ifdef IS_WINDOWS
    if (SHGetFolderPathA(NULL, CSIDL_PROFILE, NULL, 0, *home) != S_OK) {
        rc = RETURN_EPERM;
        goto cleanup;
    }
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
