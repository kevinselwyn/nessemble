#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "nessemble.h"
#include "error.h"

/**
 * Trigger fatal error
 * @param {const char *} fmt - Format string
 * @param {...} ... - Variable arguments
 */
void fatal(const char *fmt, ...) {
    int rc = RETURN_EPERM;

    va_list argptr;
    va_start(argptr, fmt);

    fprintf(stderr, "Fatal error: ");
    (void)vfprintf(stderr, fmt, argptr);
    fprintf(stderr, "\n");

    va_end(argptr);

    exit(rc);
}

/**
 * Issue a warning
 * @param {const char *} fmt - Format string
 * @param {...} ... - Variable arguments
 */
void warning(const char *fmt, ...) {
    int line = 0;
    size_t length = 0;

    va_list argptr;
    va_start(argptr, fmt);

    length = strlen(cwd_path) + 1;

    line = yylineno - 1;

    if (line <= 0) {
        line = 1;
    }

    fprintf(stderr, "Warning in `%s` on line %d: ", filename_stack[include_stack_ptr]+length, line);
    (void)vfprintf(stderr, fmt, argptr);
    fprintf(stderr, "\n");

    va_end(argptr);
}

/**
 * Add to error bus
 * @param {const char *} fmt - Format string
 * @param {...} ... - Variable arguments
 */
void error(const char *fmt, ...) {
    va_list argptr;
    va_start(argptr, fmt);

    errors[error_index].stack = (unsigned int)include_stack_ptr;
    errors[error_index].line = yylineno;

    errors[error_index].message = (char *)malloc(sizeof(char) * MAX_ERROR_LENGTH);

    if (!errors[error_index].message) {
        error_index++;
        return;
    }

    (void)vsprintf(errors[error_index++].message, fmt, argptr);

    if (error_index >= MAX_ERROR_COUNT) {
        (void)error_exit();
    }
}

/**
 * Check if has errors
 * @return {unsigned int} True if has errors, false if not
 */
unsigned int error_exists() {
    if (error_index > 0) {
        return TRUE;
    }

    return FALSE;
}

/**
 * Exit with error
 * @return {unsigned int} Error return code
 */
unsigned int error_exit() {
    unsigned int rc = RETURN_OK, i = 0, l = 0;
    int line = 0;
    size_t length = 0;

    if (error_exists() == TRUE) {
        length = strlen(cwd_path) + 1;

        for (i = 0, l = error_index; i < l; i++) {
            include_stack_ptr = (int)errors[i].stack;
            yylineno = errors[i].line;

            line = yylineno - 1;

            if (line <= 0) {
                line = 1;
            }

            fprintf(stderr, "Error in `%s` on line %d: %s\n", filename_stack[include_stack_ptr]+length, line, errors[i].message);
        }

        rc = RETURN_EPERM;
    }

    return rc;
}

/**
 * Free all errors
 */
void error_free() {
    unsigned int i = 0, l = 0;

    for (i = 0, l = error_index; i < l; i++) {
        if (errors[i].message) {
            free(errors[i].message);
        }
    }
}

/**
 * Parser error
 * @param {const char *} fmt - Format string
 * @param {...} ... - Variable arguments
 */
void yyerror(const char *fmt, ...) {
    int rc = RETURN_EPERM, line = 0;
    size_t length = 0;

    va_list argptr;
    va_start(argptr, fmt);

    length = strlen(cwd_path) + 1;

    line = yylineno - 1;

    if (line <= 0) {
        line = 1;
    }

    fprintf(stderr, "Error in `%s` on line %d: ", filename_stack[include_stack_ptr]+length, line);
    (void)vfprintf(stderr, fmt, argptr);
    fprintf(stderr, "\n");

    va_end(argptr);

    exit(rc);
}
