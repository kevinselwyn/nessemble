#include <string.h>
#include <float.h>
#include "nessemble.h"

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
