#include <string.h>
#include "../nessemble.h"

#ifndef IS_WINDOWS
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <dlfcn.h>
#endif /* IS_WINDOWS */

/**
 * custom pseudo instruction
 */
void pseudo_custom(char *pseudo) {
    unsigned int i = 0, l = 0;
    size_t return_len = 0;
    char *exec = NULL, *ext = NULL;
    char *return_str = NULL;

    if (pseudo_parse(&exec, pseudo) != RETURN_OK) {
        yyerror(_("Unknown custom pseudo-instruction `%s`"), pseudo);
        goto cleanup;
    }

    if (!exec) {
        goto cleanup;
    }

    if (parse_extension(&ext, exec) != RETURN_OK) {
        yyerror(_("Unknown extension `%s`"), pseudo);
        goto cleanup;
    }

    if (file_exists(exec) == FALSE) {
        yyerror(_("Command for custom pseudo-instruction `%s` does not exist"), pseudo);
        goto cleanup;
    }

    if (ext != NULL && strcmp(ext, "lua") == 0) {
#ifndef IS_WINDOWS
        lua_State *L;

        L = luaL_newstate();
        luaL_openlibs(L);

        if (luaL_loadfile(L, exec) != 0) {
            goto cleanup;
        }

        if (lua_pcall(L, 0, 0, 0) != 0) {
            goto cleanup;
        }

        lua_getglobal(L, "custom");

        for (i = 0, l = length_ints; i < l; i++) {
            lua_pushnumber(L, ints[i]);
        }

        if (lua_pcall(L, length_ints, 1, 0) != 0) {
            goto cleanup;
        }

        return_str = (char *)lua_tolstring(L, -1, &return_len);

        for (i = 0, l = (unsigned int)return_len; i < l; i++) {
            write_byte((unsigned int)return_str[i] & 0xFF);
        }

        lua_close(L);
#endif /* IS_WINDOWS */
    } else if (ext != NULL && strcmp(ext, "so") == 0) {
#ifndef IS_WINDOWS
        void *handle = NULL;
        int (*custom)(int **, int *, int, int *);
        int custom_length = 0;
        int *custom_output = NULL;

        handle = dlopen(exec, RTLD_LAZY);

        if (!handle) {
            goto cleanup;
        }

        custom = dlsym(handle, "custom");

        if (dlerror() != NULL) {
            goto cleanup;
        }

        if ((*custom)(&custom_output, &custom_length, (int)length_ints, (int *)ints) != 0) {
            goto cleanup;
        }

        if (!custom_output) {
            goto cleanup;
        }

        if (custom_length >= 1) {
            for (i = 0, l = (unsigned int)custom_length; i < l; i++) {
                write_byte((unsigned int)custom_output[i] & 0xFF);
            }
        }

        nessemble_free(custom_output);

        UNUSED(dlclose(handle));
#endif /* IS_WINDOWS */
    } else {
        int rc = 0;
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
            goto cleanup;
        }

        output_length = (unsigned int)fread(buffer, 1, 1024, pipe);

        for (i = 0, l = output_length; i < l; i++) {
            write_byte((unsigned int)buffer[i] & 0xFF);
        }

        if (pipe) {
            if ((rc = pclose(pipe)) != 0) {
                yyerror(_("Command for custom pseudo-instruction `%s` failed (%d)"), pseudo, rc);
                goto cleanup;
            }
        }

        nessemble_free(command);
    }

cleanup:
    nessemble_free(exec);

    length_ints = 0;
}
