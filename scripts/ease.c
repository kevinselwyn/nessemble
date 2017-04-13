#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#define EASING_COUNT 12

float easeInQuad(float t, float b, float c, float d) {
    float new_t = t / d;

    return c * new_t * new_t + b;
}

float easeOutQuad(float t, float b, float c, float d) {
    float new_t = t / d;

    return -c * new_t * (new_t - 2) + b;
}

float easeInOutQuad(float t, float b, float c, float d) {
    float new_t = t / (d / 2), lower_t = 0.0;

    if (new_t < 1) {
        return c / 2 * new_t * new_t + b;
    }

    lower_t = new_t - 1;

    return -c / 2 * (lower_t * (lower_t - 2) - 1) + b;
}

float easeInCubic(float t, float b, float c, float d) {
    float new_t = t / d;

    return c * new_t * new_t * new_t + b;
}

float easeOutCubic(float t, float b, float c, float d) {
    float new_t = (t / d) - 1;

    return c * (new_t * new_t * new_t + 1) + b;
}

float easeInOutCubic(float t, float b, float c, float d) {
    float new_t = t / (d / 2), lower_t = 0.0;

    if (new_t < 1) {
        return c / 2 * new_t * new_t * new_t + b;
    }

    lower_t = new_t - 2;

	return c / 2 * (lower_t * lower_t * lower_t + 2) + b;
}

float easeInQuint(float t, float b, float c, float d) {
    float new_t = t / d;

    return c * new_t * new_t * new_t * new_t * new_t + b;
}

float easeOutQuint(float t, float b, float c, float d) {
    float new_t = (t / d) - 1;

    return c * (new_t * new_t * new_t * new_t * new_t + 1) + b;
}

float easeInOutQuint(float t, float b, float c, float d) {
    float new_t = (t / (d / 2)), lower_t = 0.0;

    if (new_t < 1) {
        return c / 2 * new_t * new_t * new_t * new_t * new_t + b;
    }

    lower_t = new_t - 2;

	return c / 2 * (lower_t * lower_t * lower_t * lower_t * lower_t + 2) + b;
}

float easeOutBounce(float t, float b, float c, float d) {
    float new_t = t / d, lower_t = 0.0;
    float magic = 7.5625;

    if (new_t < (float)(1 / 2.75)) {
        return c * (magic * new_t * new_t) + b;
    } else if (new_t < (float)(2 / 2.75)) {
        lower_t = new_t - (float)(1.5 / 2.75);
        return c * (magic * lower_t * lower_t + (float)0.75) + b;
    } else if (new_t < (float)(2.5 / 2.75)) {
        lower_t = new_t - (float)(2.25 / 2.75);
        return c * (magic * lower_t * lower_t + (float)0.9375) + b;
    } else {
        lower_t = new_t - (float)(2.625 / 2.75);
        return c * (magic * lower_t * lower_t + (float)0.984375) + b;
    }
}

float easeInBounce(float t, float b, float c, float d) {
    return c - easeOutBounce(d - t, 0, c, d) + b;
}

float easeInOutBounce(float t, float b, float c, float d) {
    if (t < d / 2) {
        return easeInBounce(t * 2, 0, c, d) * (float)0.5 + b;
    }

    return easeOutBounce(t * 2 - d, 0, c, d) * (float)0.5 + c * (float)0.5 + b;
}

struct easing {
    char *type;
    float (*func)(float t, float b, float c, float d);
};

struct easing easings[EASING_COUNT] = {
    { "easeInQuad",      &easeInQuad },
    { "easeOutQuad",     &easeOutQuad },
    { "easeInOutQuad",   &easeInOutQuad },
    { "easeInCubic",     &easeInCubic },
    { "easeOutCubic",    &easeOutCubic },
    { "easeInOutCubic",  &easeInOutCubic },
    { "easeInQuint",     &easeInQuint },
    { "easeOutQuint",    &easeOutQuint },
    { "easeInOutQuint",  &easeInOutQuint },
    { "easeInBounce",    &easeInBounce },
    { "easeOutBounce",   &easeOutBounce },
    { "easeInOutBounce", &easeInOutBounce }
};

int custom(char **return_str, size_t *return_len, unsigned int *ints, int length_ints, char *texts[], int length_texts) {
    int rc = 0, i = 0, j = 0, k = 0, l = 0;
    int result_index = 0, func_found = 0;
    int start = 0, end = 16, steps = 16;
    unsigned int value_out = 0;
    size_t result_len = 0, ease_len = 0;
    float value = 0.0;
    char *ease = texts[0], *result = NULL;

    if (!ease) {
        result_len = 19;
        result = (char *)malloc(sizeof(char) * (result_len + 1));
        strcpy(result, "Missing easing type");

        rc = 1;
        goto cleanup;
    }

    ease++;
    ease_len = strlen(ease);

    ease[ease_len - 1] = '\0';

    for (i = 0, l = EASING_COUNT; i < l; i++) {
        if (func_found == 0 && strcmp(easings[i].type, ease) == 0) {
            func_found = 1;
            break;
        }
    }

    if (func_found == 0) {
        result_len = 22 + strlen(ease);
        result = (char *)malloc(sizeof(char) * (result_len + 1));
        sprintf(result, "Invalid easing type `%s`", ease);

        rc = 1;
        goto cleanup;
    }

    if (length_ints > 0) {
        start = ints[0];
    }

    if (length_ints > 1) {
        end = ints[1];
    }

    if (length_ints > 2) {
        steps = ints[2];
    }

    if (steps > 255) {
        steps = 255;
    }

    result_len = steps;
    result = (char *)malloc(sizeof(char) * (steps));

    for (i = 0, j = EASING_COUNT; i < j; i++) {
        if (strcmp(ease, easings[i].type) == 0) {
            for (k = 0, l = steps; k < l; k++) {
                if (end > start) {
                    value = (*easings[i].func)((float)k / ((float)l - 1), 0, 1, 1) * (end - start);
                    result[result_index++] = (char)(((unsigned int)value + start) & 0xFF);
                } else {
                    value = (*easings[i].func)((float)(l - k - 1) / ((float)l - 1), 0, 1, 1) * (start - end);
                    result[result_index++] = (char)(((unsigned int)value + end) & 0xFF);
                }
            }
        }
    }

cleanup:
    *return_str = result;
    *return_len = result_len;

    return rc;
}
