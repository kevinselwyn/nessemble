#pragma once

#include <stdint.h>
#include <errno.h>

/***** Application defined functions *****/
int udeflate_read_bits(int n_bits);
int udeflate_read_huffman_bits(int n_bits);
int udeflate_peek_huffman_bits(int n_bits); /* TODO combine with read_huffman_bits? */
int udeflate_read_next_byte();
int udeflate_write_byte(uint8_t data);
int udeflate_write_match(uint16_t len, uint16_t dist);
int udeflate_write_input_bytes(uint16_t len);

/***** Deflate functions *****/
int deflate();
