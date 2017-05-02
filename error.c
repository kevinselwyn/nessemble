#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include "nessemble.h"
#include "error.h"

#if !defined(IS_WINDOWS) && !defined(IS_JAVASCRIPT)
#include <execinfo.h>
#endif /* !IS_WINDOWS && !IS_JAVASCRIPT */

#define ERROR_BACKTRACE_SIZE 10
#define SIGNAL_COUNT         32

static char *signal_names[SIGNAL_COUNT] = {
    NULL,
    "SIGHUP",
    "SIGINT",
    "SIGQUIT",
    "SIGILL",
    "SIGTRAP",
    "SIGABRT",
    "SIGEMT",
    "SIGFPE",
    "SIGKILL",
    "SIGBUS",
    "SIGSEGV",
    "SIGSYS",
    "SIGPIPE",
    "SIGALRM",
    "SIGTERM",
    "SIGURG",
    "SIGSTOP",
    "SIGTSTP",
    "SIGCONT",
    "SIGCHLD",
    "SIGTTIN",
    "SIGTTOU",
    "SIGIO",
    "SIGXCPU,"
    "SIGXFSZ",
    "SIGVTALRM",
    "SIGPROF",
    "SIGWINCH",
    "SIGINFO",
    "SIGUSR1",
    "SIGUSR2"
};

/**
 * Trigger fatal error
 * @param {const char *} fmt - Format string
 * @param {...} ... - Variable arguments
 */
void fatal(const char *fmt, ...) {
    int rc = RETURN_EPERM;

    va_list argptr;
    va_start(argptr, fmt);

    fprintf(stderr, "%s", _("Fatal error: "));
    UNUSED(vfprintf(stderr, fmt, argptr));
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

    fprintf(stderr, _("Warning in `%s` on line %d: "), filename_stack[include_stack_ptr]+length, line);
    UNUSED(vfprintf(stderr, fmt, argptr));
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

    errors[error_index].message = (char *)nessemble_malloc(sizeof(char) * MAX_ERROR_LENGTH);

    UNUSED(vsnprintf(errors[error_index++].message, MAX_ERROR_LENGTH, fmt, argptr));

    if (error_index >= MAX_ERROR_COUNT) {
        UNUSED(error_exit());
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
    unsigned int rc = RETURN_OK;
    int line = 0;
    size_t length = 0;

    if (error_exists() == TRUE) {
        length = strlen(cwd_path) + 1;

        include_stack_ptr = (int)errors[error_index-1].stack;
        yylineno = errors[error_index-1].line;

        line = yylineno - 1;

        if (line <= 0) {
            line = 1;
        }

        fprintf(stderr, _("Error in `%s` on line %d: %s"), filename_stack[include_stack_ptr]+length, line, errors[error_index-1].message);
        fprintf(stderr, "\n");

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
        nessemble_free(errors[i].message);
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

    fprintf(stderr, _("Error in `%s` on line %d: "), filename_stack[include_stack_ptr]+length, line);
    UNUSED(vfprintf(stderr, fmt, argptr));
    fprintf(stderr, "\n");

    va_end(argptr);

    exit(rc);
}

void error_program_log(const char *fmt, ...) {
    va_list argptr;
    va_start(argptr, fmt);

    if (program_error_index >= MAX_ERROR_COUNT) {
        return;
    }

    program_errors[program_error_index].message = (char *)nessemble_malloc(sizeof(char) * MAX_ERROR_LENGTH);

    UNUSED(vsnprintf(program_errors[program_error_index++].message, MAX_ERROR_LENGTH, fmt, argptr));
}

void error_program_output(const char *fmt, ...) {
    unsigned int i = 0, l = 0;
    char *message = NULL;
    va_list argptr;
    va_start(argptr, fmt);

    if (program_error_index >= MAX_ERROR_COUNT) {
        return;
    }

    message = (char *)nessemble_malloc(sizeof(char) * MAX_ERROR_LENGTH);

    UNUSED(vsnprintf(message, MAX_ERROR_LENGTH, fmt, argptr));

    if (program_error_index > 0) {
        fprintf(stderr, "%s: %s\n", message, program_errors[program_error_index-1].message);
    } else {
        fprintf(stderr, "%s\n", message);
    }

    for (i = 0, l = program_error_index; i < l; i++) {
        nessemble_free(program_errors[i].message);
    }

    nessemble_free(message);
}

#if !defined(IS_WINDOWS) && !defined(IS_JAVASCRIPT)
static void error_signal_handler(int signal, siginfo_t *si, void *arg) {
    void *array[ERROR_BACKTRACE_SIZE];
    size_t size = 0;

    UNUSED(signal);
    UNUSED(arg);

    printf(_("An unexpected error has occurred: %s"), signal_names[si->si_signo]);

    if (si->si_addr) {
        printf(" (%p)", si->si_addr);
    }

    printf("\n\n%s\n  " PROGRAM_AUTHOR " (" PROGRAM_AUTHOR_EMAIL ")\n  " PROGRAM_ISSUES "\n\n", _("Please report this error to the maintainer:"));

    size = backtrace(array, ERROR_BACKTRACE_SIZE);
    backtrace_symbols_fd(array, size, STDERR_FILENO);

    longjmp(error_jmp, 1);
}

void error_signal() {
    unsigned int i = 0, l = 0;
    struct sigaction sa;

    memset(&sa, '\0', sizeof(struct sigaction));

    sigemptyset(&sa.sa_mask);
    sa.sa_sigaction = error_signal_handler;
    sa.sa_flags = SA_SIGINFO;

    for (i = 1, l = 32; i < l; i++) {
        if (i == 9 || i == 13 || i == 17 || i == 19 || i == 20) {
            continue;
        }

        sigaction(i, &sa, NULL);
    }
}
#endif /* !IS_WINDOWS && !IS_JAVASCRIPT */
