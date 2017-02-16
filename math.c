#include <string.h>
#include <float.h>
#include "nessemble.h"

#define CRC_TABLE_SIZE 256

/**
 * Calculate CRC-32
 * @param {unsigned int *} buffer - Data buffer
 * @param {unsigned int} length - Data length
 * @return {unsigned int} CRC-32 value
 */
unsigned int crc_32(unsigned int *buffer, unsigned int length) {
    unsigned int crc = 0;
    unsigned int has_table = FALSE, rem = 0, octet = 0, i = 0, j = 0;
    unsigned int table[CRC_TABLE_SIZE];
    unsigned int *p, *q;

    memset(table, 0, CRC_TABLE_SIZE);

    if (has_table == FALSE) {
        for (i = 0; i < 256; i++) {
            rem = i;

            for (j = 0; j < 8; j++) {
                if ((rem & 1) != 0) {
                    rem >>= 1;
                    rem ^= 0xEDB88320;
                } else {
                    rem >>= 1;
                }
            }

            table[i] = rem;
        }

        has_table = TRUE;
    }

    crc = ~crc;
    q = buffer + length;

    for (p = buffer; p < q; p++) {
        octet = *p;
        crc = (crc >> 8) ^ table[(crc & 0xFF) ^ octet];
    }

    return ~crc;
}

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

/*
 * Ease In Quad
 * @param {float} t - Current time
 * @param {float} b - Beginning value
 * @param {float} c - Change in value
 * @param {float} d - Duration
 * @return {float} Eased value
 */
float easeInQuad(float t, float b, float c, float d) {
    float new_t = t / d;

    return c * new_t * new_t + b;
}

/*
 * Ease Out Quad
 * @param {float} t - Current time
 * @param {float} b - Beginning value
 * @param {float} c - Change in value
 * @param {float} d - Duration
 * @return {float} Eased value
 */
float easeOutQuad(float t, float b, float c, float d) {
    float new_t = t / d;

    return -c * new_t * (new_t - 2) + b;
}

/*
 * Ease In Out Quad
 * @param {float} t - Current time
 * @param {float} b - Beginning value
 * @param {float} c - Change in value
 * @param {float} d - Duration
 * @return {float} Eased value
 */
float easeInOutQuad(float t, float b, float c, float d) {
    float new_t = t / (d / 2), lower_t = 0.0;

    if (new_t < 1) {
        return c / 2 * new_t * new_t + b;
    }

    lower_t = new_t - 1;

    return -c / 2 * (lower_t * (lower_t - 2) - 1) + b;
}

/*
 * Ease In Cubic
 * @param {float} t - Current time
 * @param {float} b - Beginning value
 * @param {float} c - Change in value
 * @param {float} d - Duration
 * @return {float} Eased value
 */
float easeInCubic(float t, float b, float c, float d) {
    float new_t = t / d;

    return c * new_t * new_t * new_t + b;
}

/*
 * Ease Out Cubic
 * @param {float} t - Current time
 * @param {float} b - Beginning value
 * @param {float} c - Change in value
 * @param {float} d - Duration
 * @return {float} Eased value
 */
float easeOutCubic(float t, float b, float c, float d) {
    float new_t = (t / d) - 1;

    return c * (new_t * new_t * new_t + 1) + b;
}

/*
 * Ease In Out Cubic
 * @param {float} t - Current time
 * @param {float} b - Beginning value
 * @param {float} c - Change in value
 * @param {float} d - Duration
 * @return {float} Eased value
 */
float easeInOutCubic(float t, float b, float c, float d) {
    float new_t = t / (d / 2), lower_t = 0.0;

    if (new_t < 1) {
        return c / 2 * new_t * new_t * new_t + b;
    }

    lower_t = new_t - 2;

	return c / 2 * (lower_t * lower_t * lower_t + 2) + b;
}

/*
 * Ease In Quint
 * @param {float} t - Current time
 * @param {float} b - Beginning value
 * @param {float} c - Change in value
 * @param {float} d - Duration
 * @return {float} Eased value
 */
float easeInQuint(float t, float b, float c, float d) {
    float new_t = t / d;

    return c * new_t * new_t * new_t * new_t * new_t + b;
}

/*
 * Ease Out Quint
 * @param {float} t - Current time
 * @param {float} b - Beginning value
 * @param {float} c - Change in value
 * @param {float} d - Duration
 * @return {float} Eased value
 */
float easeOutQuint(float t, float b, float c, float d) {
    float new_t = (t / d) - 1;

    return c * (new_t * new_t * new_t * new_t * new_t + 1) + b;
}

/*
 * Ease In Out Quint
 * @param {float} t - Current time
 * @param {float} b - Beginning value
 * @param {float} c - Change in value
 * @param {float} d - Duration
 * @return {float} Eased value
 */
float easeInOutQuint(float t, float b, float c, float d) {
    float new_t = (t / (d / 2)), lower_t = 0.0;

    if (new_t < 1) {
        return c / 2 * new_t * new_t * new_t * new_t * new_t + b;
    }

    lower_t = new_t - 2;

	return c / 2 * (lower_t * lower_t * lower_t * lower_t * lower_t + 2) + b;
}

/*
 * Ease In Bounce
 * @param {float} t - Current time
 * @param {float} b - Beginning value
 * @param {float} c - Change in value
 * @param {float} d - Duration
 * @return {float} Eased value
 */
float easeInBounce(float t, float b, float c, float d) {
    return c - easeOutBounce(d - t, 0, c, d) + b;
}

/*
 * Ease Out Bounce
 * @param {float} t - Current time
 * @param {float} b - Beginning value
 * @param {float} c - Change in value
 * @param {float} d - Duration
 * @return {float} Eased value
 */
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

/*
 * Ease In Out Bounce
 * @param {float} t - Current time
 * @param {float} b - Beginning value
 * @param {float} c - Change in value
 * @param {float} d - Duration
 * @return {float} Eased value
 */
float easeInOutBounce(float t, float b, float c, float d) {
    if (t < d / 2) {
        return easeInBounce(t * 2, 0, c, d) * (float)0.5 + b;
    }

    return easeOutBounce(t * 2 - d, 0, c, d) * (float)0.5 + c * (float)0.5 + b;
}
