#include <string.h>
#include "../nessemble.h"

#ifndef IS_JAVASCRIPT
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
#endif

unsigned int scripting_lua(char *exec, char **output) {
    unsigned int rc = RETURN_OK;

#ifndef IS_JAVASCRIPT
    int has_error = 0;
    unsigned int i = 0, l = 0;
    size_t return_len = 0, path_len = 0;
    char *return_str = NULL, *error_str = NULL;
    char *path = NULL, *home_path = NULL, *new_path = NULL;

    lua_State *L;

    L = luaL_newstate();
    luaL_openlibs(L);

    // set package path

    lua_getglobal(L, "package");
    lua_getfield(L, -1, "path");
    path = (char *)lua_tolstring(L, -1, &path_len);

    if ((rc = get_home_path(&home_path, 2, "." PROGRAM_NAME, "scripts" SEP "?.lua")) != RETURN_OK) {
        goto cleanup;
    }

    path_len += strlen(home_path) + 1;

    new_path = (char *)nessemble_malloc(sizeof(char) * (path_len + 1));
    sprintf(new_path, "%s;%s", path, home_path);

    lua_pop(L, 1);
    lua_pushstring(L, new_path);
    lua_setfield(L, -2, "path");
    lua_pop(L, 1);

    // load file

    if ((has_error = luaL_loadfile(L, exec)) != 0) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    if ((has_error = lua_pcall(L, 0, 0, 0)) != 0) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    for (i = 0, l = length_texts; i < l; i++) {
        lua_getglobal(L, "add_string");

        if (!lua_isnil(L, -1)) {
            lua_pushstring(L, texts[i]);

            if ((has_error = lua_pcall(L, 1, 0, 0) != 0)) {
                rc = RETURN_EPERM;
                goto cleanup;
            }
        }
    }

    lua_getglobal(L, "custom");

    for (i = 0, l = length_ints; i < l; i++) {
        lua_pushnumber(L, ints[i]);
    }

    if ((has_error = lua_pcall(L, length_ints, 1, 0)) != 0) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    return_str = (char *)lua_tolstring(L, -1, &return_len);

    for (i = 0, l = (unsigned int)return_len; i < l; i++) {
        write_byte((unsigned int)return_str[i] & 0xFF);
    }

cleanup:
    if (has_error != 0) {
        error_str = strchr(lua_tostring(L, -1), ':');
        error_str = strchr(error_str+2, ':');
        error_str += 2;

        return_len = strlen(error_str);

        *output = (char *)nessemble_malloc(sizeof(char) * (return_len + 1));
        strcpy(*output, error_str);
    }

    lua_close(L);

    nessemble_free(home_path);
    nessemble_free(new_path);

#endif /* IS_JAVASCRIPT */
    return rc;
}
