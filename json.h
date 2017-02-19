#ifndef _JSON_H
#define _JSON_H

typedef enum {
    JSON_UNDEFINED,
    JSON_NUMBER,
    JSON_STRING,
    JSON_BOOLEAN,
    JSON_ARRAY,
    JSON_OBJECT,
    JSON_NULL
} json_type;

struct json_token {
    json_type type;
    unsigned int key, start, end, size;
};

#endif /* _JSON_H */
