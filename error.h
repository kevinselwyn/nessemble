#ifndef _ERROR_H
#define _ERROR_H

#define MAX_ERROR_COUNT  10
#define MAX_ERROR_LENGTH 256

struct error {
    unsigned int stack;
    int line;
    char *message;
};

static struct error errors[MAX_ERROR_COUNT];

static unsigned int error_index = 0;

#endif /* _ERROR_H */
