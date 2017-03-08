#include <string.h>
#include <unistd.h>
#include "nessemble.h"

#ifdef WIN32
#include <windows.h>
#include <shlobj.h>
#else /* WIN32 */
#include <pwd.h>
#endif /* WIN32 */

unsigned int get_home(char **home) {
    unsigned int rc = RETURN_OK;

#ifdef WIN32
    if (SHGetFolderPathA(NULL, CSIDL_PROFILE, NULL, 0, *home) != S_OK) {
        rc = RETURN_EPERM;
        goto cleanup;
    }
#else /* WIN32 */
    struct passwd *pw = getpwuid(getuid());

    if (!pw) {
        error_program_log("Could not find home directory");

        rc = RETURN_EPERM;
        goto cleanup;
    }

    *home = (char *)nessemble_malloc(sizeof(char) * (strlen(pw->pw_dir) + 1));

    strcpy(*home, pw->pw_dir);
#endif /* WIN32*/

cleanup:
    return rc;
}
