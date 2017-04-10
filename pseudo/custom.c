#include <string.h>
#include "../nessemble.h"
#include "../third-party/duktape/duktape.h"

#if !defined(IS_WINDOWS) && !defined(IS_JAVASCRIPT)
#ifdef IS_MAC
#include <Python/Python.h>
#else /* IS_MAC */
#include <Python.h>
#endif /* IS_MAC */
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include <dlfcn.h>
#endif /* !IS_WINDOWS && !IS_JAVASCRIPT */

/**
 * custom pseudo instruction
 */
void pseudo_custom(char *pseudo) {
    int error = 0;
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

    if (ext != NULL && strcmp(ext, "js") == 0) {
        unsigned int exec_len = 0;
        char *exec_data = NULL;

        duk_context *ctx = duk_create_heap_default();

        if (load_file(&exec_data, &exec_len, exec) != RETURN_OK) {
            yyerror(_("Could not load file `%s`"), exec);
            goto cleanup_js;
        }

        duk_push_string(ctx, exec_data);
        duk_eval(ctx);

        duk_get_global_string(ctx, "custom");

        for (i = 0, l = length_ints; i < l; i++) {
            duk_push_int(ctx, ints[i]);
        }

        if (duk_pcall(ctx, length_ints) != 0) {
            if (strlen(exec) > 56) {
                fprintf(stderr, "...%.56s", exec+(strlen(exec)-56));
            } else {
                fprintf(stderr, "%s", exec);
            }

            fprintf(stderr, ": %s\n", duk_safe_to_string(ctx, -1));
            goto cleanup_js;
        }

        return_str = (char *)duk_get_string(ctx, -1);
        return_len = (size_t)duk_get_length(ctx, -1);

        for (i = 0, l = (unsigned int)return_len; i < l; i++) {
            write_byte((unsigned int)return_str[i] & 0xFF);
        }

cleanup_js:
        duk_destroy_heap(ctx);
        nessemble_free(exec_data);
#if !defined(IS_WINDOWS) && !defined(IS_JAVASCRIPT)
    } else if (ext != NULL && strcmp(ext, "py") == 0) {
        unsigned int exec_len = 0;
        char *exec_data = NULL;
        PyObject *pMainString, *pMain, *pFunc, *pArgs, *pResult, *pValue;

        Py_Initialize();

        pMainString = PyString_FromString("__main__");
        pMain = PyImport_Import(pMainString);

        if (load_file(&exec_data, &exec_len, exec) != RETURN_OK) {
            yyerror(_("Could not load file `%s`"), exec);
            goto cleanup_py;
        }

        PyRun_SimpleString(exec_data);

        pFunc = PyObject_GetAttrString(pMain, "custom");

        if (!pFunc) {
            if (strlen(exec) > 56) {
                fprintf(stderr, "...%.56s", exec+(strlen(exec)-56));
            } else {
                fprintf(stderr, "%s", exec);
            }

            fprintf(stderr, ": Can't fetch function `custom`\n");

            goto cleanup_py;
        }

        pArgs = PyTuple_New(length_ints);

        for (i = 0, l = length_ints; i < l; i++) {
            pValue = PyInt_FromLong(ints[i]);
            PyTuple_SetItem(pArgs, i, pValue);
        }

        pResult = PyObject_CallObject(pFunc, pArgs);

        long int py_return_len = 0;
        PyString_AsStringAndSize(pResult, &return_str, &py_return_len);

        for (i = 0, l = (unsigned int)py_return_len; i < l; i++) {
            write_byte((unsigned int)return_str[i]);
        }

cleanup_py:
        Py_Finalize();
        nessemble_free(exec_data);
    } else if (ext != NULL && strcmp(ext, "lua") == 0) {
        lua_State *L;

        L = luaL_newstate();
        luaL_openlibs(L);

        if ((error = luaL_loadfile(L, exec)) != 0) {
            goto cleanup_lua;
        }

        if ((error = lua_pcall(L, 0, 0, 0)) != 0) {
            goto cleanup_lua;
        }

        lua_getglobal(L, "custom");

        for (i = 0, l = length_ints; i < l; i++) {
            lua_pushnumber(L, ints[i]);
        }

        if ((error = lua_pcall(L, length_ints, 1, 0)) != 0) {
            goto cleanup_lua;
        }

        return_str = (char *)lua_tolstring(L, -1, &return_len);

        for (i = 0, l = (unsigned int)return_len; i < l; i++) {
            write_byte((unsigned int)return_str[i] & 0xFF);
        }

cleanup_lua:
        if (error != 0) {
            fprintf(stderr, "%s\n", lua_tostring(L, -1));
        }

        lua_close(L);
    } else if (ext != NULL && strcmp(ext, "so") == 0) {
        void *handle = NULL;
        int (*custom)(char **, size_t *, unsigned int *, int);

        handle = dlopen(exec, RTLD_LAZY);

        if (!handle) {
            goto cleanup_so;
        }

        custom = dlsym(handle, "custom");

        if (dlerror() != NULL) {
            if (strlen(exec) > 56) {
                fprintf(stderr, "...%.56s", exec+(strlen(exec)-56));
            } else {
                fprintf(stderr, "%s", exec);
            }

            fprintf(stderr, ": Can't fetch function `custom`\n");

            goto cleanup_so;
        }

        if ((*custom)(&return_str, &return_len, ints, length_ints) != 0) {
            goto cleanup_so;
        }

        if (!return_str) {
            goto cleanup_so;
        }

        if (return_len >= 1) {
            for (i = 0, l = (unsigned int)return_len; i < l; i++) {
                write_byte((unsigned int)return_str[i] & 0xFF);
            }
        }

cleanup_so:
        nessemble_free(return_str);
        UNUSED(dlclose(handle));
#endif /* !IS_WINDOWS && !IS_JAVSCRIPT */
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
    nessemble_free(ext);

    length_ints = 0;
}
