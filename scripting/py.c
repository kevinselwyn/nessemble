#include "../nessemble.h"

#if !defined(IS_WINDOWS) && !defined(IS_JAVASCRIPT)
#ifdef IS_MAC
#include <Python/Python.h>
#else /* IS_MAC */
#include <Python.h>
#endif /* IS_MAC */
#endif /* !IS_WINDOWS && !IS_JAVASCRIPT */

unsigned int scripting_py(char *exec) {
    unsigned int rc = RETURN_OK, i = 0, l = 0;
    unsigned int exec_len = 0;
    long int return_len = 0;
    char *exec_data = NULL, *return_str = NULL;
    PyObject *pMainString, *pMain, *pFunc, *pArgs, *pResult, *pValue;

    Py_Initialize();

    pMainString = PyString_FromString("__main__");
    pMain = PyImport_Import(pMainString);

    if (load_file(&exec_data, &exec_len, exec) != RETURN_OK) {
        yyerror(_("Could not load file `%s`"), exec);

        rc = RETURN_EPERM;
        goto cleanup;
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

        rc = RETURN_EPERM;
        goto cleanup;
    }

    pArgs = PyTuple_New(length_ints);

    for (i = 0, l = length_ints; i < l; i++) {
        pValue = PyInt_FromLong(ints[i]);
        PyTuple_SetItem(pArgs, i, pValue);
    }

    pResult = PyObject_CallObject(pFunc, pArgs);

    PyString_AsStringAndSize(pResult, &return_str, &return_len);

    for (i = 0, l = (unsigned int)return_len; i < l; i++) {
        write_byte((unsigned int)return_str[i]);
    }

cleanup:
    Py_Finalize();
    nessemble_free(exec_data);

    return rc;
}
