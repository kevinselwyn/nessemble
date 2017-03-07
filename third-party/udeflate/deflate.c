/******************************************************************************

MIT License

Copyright (c) 2017 Joachim Lublin

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

******************************************************************************/

#include "deflate.h"

/*
 * Error codes:
 *
 * EINVAL    - Is returned when input data is invalid
 * x         - Any code returned from I/O code
 */

/***** General functions *****/
const int EOB = 0x10000;
static int decode_symbol(uint16_t code)
{
	int ret = 0;

	if(code < 256)
	{
		udeflate_write_byte(code);
	}

	else if(code == 256)
	{
		ret = EOB;
	}
	else if(code < 265)
	{
		uint16_t len = code - 254;

		ret = len;
	}
	else if(code < 285)
	{
		uint16_t len = 3 + 4*(1<<((code-261)/4)) + ((code-1)&3)*(1<<((code-261)/4));
		len += udeflate_read_bits((code - 261) / 4);

		ret = len;
	}
	else if(code == 285)
	{
		uint16_t len = 258;

		ret = len;
	}
	else
	{
		ret = -EINVAL;
	}

	return ret;
}

/***** Non-compressed block *****/

static int read_non_compressed_block()
{
	uint8_t len = udeflate_read_next_byte();
	uint8_t nlen = udeflate_read_next_byte();

    nlen = ~nlen;

	if(len != nlen)
		return -EINVAL;

	return udeflate_write_input_bytes(len);
}

/***** Fixed Huffman coding block *****/

static int read_fixed_code()
{
	int ret;
	uint16_t code;

	if((ret = udeflate_read_huffman_bits(7)) < 0)
		return ret;

	code = ret;

	if(code < 0x18)
		return code + 256;

	if((ret = udeflate_read_huffman_bits(1)) < 0)
		return ret;

	code <<= 1;
	code |= ret;

	if(code < 0xc0)
		return code - 0x30;

	if(code < 0xc8)
		return code + 0x58;

	if((ret = udeflate_read_huffman_bits(1)) < 0)
		return ret;

	code <<= 1;
	code |= ret;

	return code - 0x100;
}

static int read_fixed_distance()
{
	int ret;
	uint16_t code;

	if((ret = udeflate_read_huffman_bits(5)) < 0)
		return ret;

	code = ret;

	if(code < 4)
		return code + 1;

	uint16_t base = 1+2*(1<<((code-2)/2)) + (code&1)*(1<<((code-2)/2));

	return base + udeflate_read_bits((code-2)/2);
}

static int read_fixed_block()
{
	while(1)
	{
		int ret;
		uint16_t code;
		uint16_t len;

		if((ret = read_fixed_code()) < 0)
			return ret;

		code = ret;

		if((ret = decode_symbol(code)) < 0)
			return ret;

		if(ret < 0)
			return len;

		if(ret == EOB)
			break;

		len = ret;

		if(len != 0)
		{
			if((ret = read_fixed_distance()) < 0)
				return ret;

			uint16_t distance = ret;

			if((ret = udeflate_write_match(len, distance)) < 0)
				return ret;
		}
	}

	return 0;
}

/***** Dynamic Huffman coding block *****/

static uint8_t code_order[19] =
{
	16, 17, 18, 0, 8, 7, 9, 6, 10,
	5, 11, 4, 12, 3, 13, 2, 14, 1, 15
};

static void build_code_tree(uint8_t code_lens[], uint16_t huffman_tree[], int n_codes, int n_bits)
{
    int i = 0, j = 0;

	/* [symbol] => <code,bits> */
	/* TODO: implement binary tree as alternative */

	uint16_t next = 0;

	for(i = 1; i < n_bits; i++)
	{
		for(j = 0; j < n_codes; j++)
		{
			if(code_lens[j] == i)
			{
				huffman_tree[2*j] = next;
				huffman_tree[2*j + 1] = i;
				next += 1;
			}
		}
		next <<= 1;
	}
}

static int read_code_tree(uint16_t code_tree[], int n_codes, int n_bits)
{
	int ret, i = 0, j = 0;
	uint16_t code;

	/* Read n bits without discarding them from input stream */
	if((ret = udeflate_peek_huffman_bits(n_bits)) < 0)
		return ret;

	code = ret;

	/* Check if code matches code_tree[2*j] */
	for(j = 0; j < n_codes; j++)
	{
		i = code_tree[2*j+1];

		if((code >> (n_bits-i)) == code_tree[2*j] && i > 0)
		{
			/* We found the code */
			/* Discard the i bits from input stream */
			if((ret = udeflate_read_huffman_bits(i)) < 0)
				return ret;

			return j;
		}
	}

	/* Error, code was not in tree */
	return -EINVAL;
}

static int read_litlen_code(uint16_t code_tree[], uint8_t *len)
{
	int ret;
	uint16_t code;

	if((ret = read_code_tree(code_tree, 19, 7)) < 0)
	{
		return ret;
	}

	code = ret;

	if(code >= 16)
    {
		if(code == 16)
        {
    		if((ret = udeflate_read_bits(2)) < 0)
    			return ret;

    		*len = ret + 3;
    	}
    	else if(code == 17)
    	{
    		if((ret = udeflate_read_bits(3)) < 0)
    			return ret;

    		*len = ret + 3;
    	}
    	else
    	{
    		if((ret = udeflate_read_bits(7)) < 0)
    			return ret;

    		*len = ret + 11;
    	}
    }

	return code;
}

static int read_dynamic_distance(uint16_t dist_tree[])
{
	int ret;
	uint16_t code;

	if((ret = read_code_tree(dist_tree, 32, 16)) < 0)
		return ret;

	code = ret;

	if(code < 4)
		return code + 1;

	uint16_t base = 1+2*(1<<((code-2)/2)) + (code&1)*(1<<((code-2)/2));

	return base + udeflate_read_bits((code-2)/2);
}

static int read_huffman_tree_lens(uint16_t cl_tree[], uint8_t code_lens[], int tree_len)
{
    int i = 0, j = 0;

	for(i = 0; i < tree_len;)
	{
		int ret;
		uint8_t len;
		uint16_t code_len;

		if((ret = read_litlen_code(cl_tree, &len)) < 0)
			return ret;

		code_len = ret;

		if(code_len < 16)
			code_lens[i++] = code_len;

		else if(code_len == 16)
			for(j = 0; j < len; j++, i++)
				code_lens[i] = code_lens[i-1];

		else
			for(j = 0; j < len; j++, i++)
				code_lens[i] = 0;
	}

    return 0;
}

static int read_dynamic_block()
{
	int ret, i = 0;
	uint16_t hlit;
	uint8_t hdist;
	uint8_t hclen;

	if((ret = udeflate_read_bits(5)) < 0)
		return ret;

	hlit = ret + 257;

	if((ret = udeflate_read_bits(5)) < 0)
		return ret;

	hdist = ret + 1;

	if((ret = udeflate_read_bits(4)) < 0)
		return ret;

	hclen = ret + 4;

	uint16_t cl_tree[19*2] = {0}; /* CL tree */
	uint16_t litlen_tree[286*2] = {0}; /* litlen tree */
	uint16_t dist_tree[32*2] = {0}; /* dist tree */

	/* Setup CL tree */
	{
		uint8_t cl_lens[19] = {0};

		/* Read CL lengths */
		for(i = 0; i < hclen; i++)
		{
			if((ret = udeflate_read_bits(3)) < 0)
				return ret;

			uint8_t code = ret;
			cl_lens[code_order[i]] = code;
		}

		/* Build CL tree */

		build_code_tree(cl_lens, cl_tree, 19, 8);
	}

	/* Setup litlen and dist trees */
	{
		uint8_t litlen_lens[286] = {0};
		uint8_t dist_lens[32] = {0};

		/* Read literal and dist tree lengths */

		if((ret = read_huffman_tree_lens(cl_tree, litlen_lens, hlit)) < 0)
			return ret;
		if((ret = read_huffman_tree_lens(cl_tree, dist_lens, hdist)) < 0)
			return ret;

		/* Build litlen and dist trees */

		build_code_tree(litlen_lens, litlen_tree, 286, 16);
		build_code_tree(dist_lens, dist_tree, 32, 16);
	}

	/* Read data */

	while(1)
	{
		if((ret = read_code_tree(litlen_tree, 286, 16)) < 0)
			return ret;

		uint16_t code = ret;

		if((ret = decode_symbol(code)) < 0)
			return ret;

		if(ret == EOB)
			break;

		uint16_t len = ret;

		if(len != 0)
		{
			if((ret = read_dynamic_distance(dist_tree)) < 0)
				return ret;

			uint16_t distance = ret;

			if((ret = udeflate_write_match(len, distance)) < 0)
				return ret;
		}
	}

	return 0;
}

/***** Main code *****/

int deflate()
{
	while(1)
	{
		int ret;

		/* Read DEFLATE header */
		if((ret = udeflate_read_bits(1)) < 0)
			return ret;

		int last_block = ret;

		if((ret = udeflate_read_bits(2)) < 0)
			return ret;

		int btype = ret;

		if(btype == 0)
		{
			if((ret = read_non_compressed_block()) < 0)
				return ret;
		}
		else if(btype == 1)
		{
			if((ret = read_fixed_block()) < 0)
				return ret;
		}
		else if(btype == 2)
		{
			if((ret = read_dynamic_block()) < 0)
				return ret;
		}
		else
		{
			return -EINVAL;
		}

		if(last_block)
			break;
	}

	return 0;
}
