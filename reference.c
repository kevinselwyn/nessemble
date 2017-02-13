#include <stdio.h>
#include <string.h>
#include "nessemble.h"
#include "reference.h"

static int get_category_id(char *category) {
    int id = -1;
    unsigned int i = 0, l = 0;

    for (i = 0, l = MAX_CATEGORIES; i < l; i++) {
        if (!categories[i].name) {
            continue;
        }

        if (strcmp(categories[i].name, category) == 0) {
            id = (int)i;
            break;
        }
    }

    return id;
}

static int get_term_id(char *term, char *category) {
    int id = -1, category_id = -1;
    unsigned int i = 0, l = 0;

    category_id = get_category_id(category);

    if (category_id == -1) {
        goto cleanup;
    }

    for (i = 0, l = MAX_TERMS; i < l; i++) {
        if (!categories[category_id].terms[i].name) {
            continue;
        }

        if (strcmp(categories[category_id].terms[i].name, term) == 0) {
            id = (int)i;
            break;
        }
    }

cleanup:
    return id;
}

unsigned int reference(char *category, char *term) {
    int category_id = -1, term_id = -1;
    unsigned int rc = RETURN_OK, i = 0, l = 0, len = 0;

    if (!category) {
        printf("Categories:\n");

        for (i = 0, l = MAX_CATEGORIES; i < l; i++) {
            if (!categories[i].name) {
                continue;
            }

            printf("  %s\n", categories[i].name);
        }

        goto cleanup;
    }

    category_id = get_category_id(category);

    if (category_id == -1) {
        printf("Unknown reference category `%s`\n", category);

        rc = RETURN_EPERM;
        goto cleanup;
    }

    if (!term) {
        printf("Terms in %s\n", category);

        for (i = 0, l = MAX_TERMS; i < l; i++) {
            if (!categories[category_id].terms[i].name) {
                continue;
            }

            printf("  %s\n", categories[category_id].terms[i].name);
        }

        goto cleanup;
    }

    term_id = get_term_id(term, category);

    if (term_id == -1) {
        printf("Unknown `%s` term `%s`\n", category, term);

        rc = RETURN_EPERM;
        goto cleanup;
    }

    len = *categories[category_id].terms[term_id].len;
    categories[category_id].terms[term_id].description[len-1] = '\0';

    printf("%s\n", (char *)categories[category_id].terms[term_id].description);

cleanup:
    return rc;
}
