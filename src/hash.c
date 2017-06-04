#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "nessemble.h"

#define rol(value, bits) (((value) << (bits)) | ((value) >> (32 - (bits))))

#define blk0(i) (block->l[i] = (rol(block->l[i], 24) & 0xFF00FF00) | (rol(block->l[i], 8) & 0x00FF00FF))
#define blk(i)  (block->l[i & 15] = rol(block->l[(i + 13) & 15] ^ block->l[(i + 8) & 15] ^ block->l[(i + 2) & 15] ^ block->l[i & 15], 1))

#define R0(v,w,x,y,z,i) z += ((w & (x ^ y)) ^ y) + blk0(i) + 0x5A827999 + rol(v, 5); w = rol(w, 30);
#define R1(v,w,x,y,z,i) z += ((w & (x ^ y)) ^ y) + blk(i) + 0x5A827999 + rol(v, 5); w = rol(w, 30);
#define R2(v,w,x,y,z,i) z += (w ^ x ^ y) + blk(i) + 0x6ED9EBA1 + rol(v, 5); w = rol(w, 30);
#define R3(v,w,x,y,z,i) z += (((w | x) & y) | (w & x)) + blk(i) + 0x8F1BBCDC+ rol(v, 5); w = rol(w, 30);
#define R4(v,w,x,y,z,i) z += (w ^ x ^ y) + blk(i) + 0xCA62C1D6 + rol(v, 5); w = rol(w, 30);

typedef struct {
    unsigned int state[5];
    unsigned int count[2];
    char buffer[64];
} SHA1_CTX;

typedef union {
    unsigned char c[64];
    unsigned int l[16];
} SHA1_BLOCK;

static void SHA1Transform(unsigned int state[5], char *buffer) {
    unsigned int a = 0, b = 0, c = 0, d = 0, e = 0;

    SHA1_BLOCK block[1];
    memcpy(block, buffer, 64);

    a = state[0];
    b = state[1];
    c = state[2];
    d = state[3];
    e = state[4];

    R0(a,b,c,d,e, 0); R0(e,a,b,c,d, 1); R0(d,e,a,b,c, 2); R0(c,d,e,a,b, 3);
    R0(b,c,d,e,a, 4); R0(a,b,c,d,e, 5); R0(e,a,b,c,d, 6); R0(d,e,a,b,c, 7);
    R0(c,d,e,a,b, 8); R0(b,c,d,e,a, 9); R0(a,b,c,d,e,10); R0(e,a,b,c,d,11);
    R0(d,e,a,b,c,12); R0(c,d,e,a,b,13); R0(b,c,d,e,a,14); R0(a,b,c,d,e,15);
    R1(e,a,b,c,d,16); R1(d,e,a,b,c,17); R1(c,d,e,a,b,18); R1(b,c,d,e,a,19);
    R2(a,b,c,d,e,20); R2(e,a,b,c,d,21); R2(d,e,a,b,c,22); R2(c,d,e,a,b,23);
    R2(b,c,d,e,a,24); R2(a,b,c,d,e,25); R2(e,a,b,c,d,26); R2(d,e,a,b,c,27);
    R2(c,d,e,a,b,28); R2(b,c,d,e,a,29); R2(a,b,c,d,e,30); R2(e,a,b,c,d,31);
    R2(d,e,a,b,c,32); R2(c,d,e,a,b,33); R2(b,c,d,e,a,34); R2(a,b,c,d,e,35);
    R2(e,a,b,c,d,36); R2(d,e,a,b,c,37); R2(c,d,e,a,b,38); R2(b,c,d,e,a,39);
    R3(a,b,c,d,e,40); R3(e,a,b,c,d,41); R3(d,e,a,b,c,42); R3(c,d,e,a,b,43);
    R3(b,c,d,e,a,44); R3(a,b,c,d,e,45); R3(e,a,b,c,d,46); R3(d,e,a,b,c,47);
    R3(c,d,e,a,b,48); R3(b,c,d,e,a,49); R3(a,b,c,d,e,50); R3(e,a,b,c,d,51);
    R3(d,e,a,b,c,52); R3(c,d,e,a,b,53); R3(b,c,d,e,a,54); R3(a,b,c,d,e,55);
    R3(e,a,b,c,d,56); R3(d,e,a,b,c,57); R3(c,d,e,a,b,58); R3(b,c,d,e,a,59);
    R4(a,b,c,d,e,60); R4(e,a,b,c,d,61); R4(d,e,a,b,c,62); R4(c,d,e,a,b,63);
    R4(b,c,d,e,a,64); R4(a,b,c,d,e,65); R4(e,a,b,c,d,66); R4(d,e,a,b,c,67);
    R4(c,d,e,a,b,68); R4(b,c,d,e,a,69); R4(a,b,c,d,e,70); R4(e,a,b,c,d,71);
    R4(d,e,a,b,c,72); R4(c,d,e,a,b,73); R4(b,c,d,e,a,74); R4(a,b,c,d,e,75);
    R4(e,a,b,c,d,76); R4(d,e,a,b,c,77); R4(c,d,e,a,b,78); R4(b,c,d,e,a,79);

    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;
    state[4] += e;

    a = b = c = d = e = 0;

    memset(block, '\0', 64);
}

void SHA1Init(SHA1_CTX* context) {
    context->state[0] = 0x67452301;
    context->state[1] = 0xEFCDAB89;
    context->state[2] = 0x98BADCFE;
    context->state[3] = 0x10325476;
    context->state[4] = 0xC3D2E1F0;
    context->count[0] = context->count[1] = 0;
}

static void SHA1Update(SHA1_CTX* context, char *data, size_t data_len) {
    unsigned int i = 0, j = 0;

    j = context->count[0];

    if ((context->count[0] += data_len << 3) < j) {
        context->count[1]++;
    }

    context->count[1] += (data_len >> 29);
    j = (j >> 3) & 63;

    if ((j + data_len) > 63) {
        memcpy(&context->buffer[j], data, (i = 64 - j));
        SHA1Transform(context->state, context->buffer);

        for (; i + 63 < data_len; i += 64) {
            SHA1Transform(context->state, &data[i]);
        }

        j = 0;
    } else {
        i = 0;
    }

    memcpy(&context->buffer[j], &data[i], data_len - i);
}

static void SHA1Final(char **output, SHA1_CTX *context) {
    unsigned int i = 0;
    char c, finalcount[8];
    char *digest = NULL;

    digest = (char *)nessemble_malloc(sizeof(char) * 21);
    memset(digest, '\0', 21);

    for (i = 0; i < 8; i++) {
        finalcount[i] = (unsigned char)((context->count[(i >= 4 ? 0 : 1)] >> ((3 - (i & 3)) * 8)) & 255);
    }

    c = 0200;

    SHA1Update(context, &c, 1);

    while ((context->count[0] & 504) != 448) {
        c = 0000;
        SHA1Update(context, &c, 1);
    }

    SHA1Update(context, finalcount, 8);

    for (i = 0; i < 20; i++) {
        digest[i] = (char)((context->state[i >> 2] >> ((3 - (i & 3)) * 8)) & 255);
    }

    memset(context, '\0', 92);
    memset(&finalcount, '\0', 8);

    *output = digest;
}

static void digest2hex(char **hex, size_t hex_len, char *digest) {
    unsigned int i = 0, l = 0, index = 0, val = 0;
    char chars[16] = "0123456789abcdef";
    char *output = NULL;

    output = (char *)nessemble_malloc(sizeof(char) * ((hex_len * 2) + 1));

    for (i = 0, l = (unsigned int)hex_len; i < l; i++) {
        val = digest[i] & 0xFF;

        output[index++] = chars[(val >> 4) & 0x0F];
        output[index++] = chars[val & 0x0F];
    }

    *hex = output;
}

void hash(char **hex, char *data, size_t data_len) {
    char *digest = NULL;
    SHA1_CTX context;

    SHA1Init(&context);
    SHA1Update(&context, data, data_len);
    SHA1Final(&digest, &context);

    digest2hex(&*hex, 20, digest);

    nessemble_free(digest);
}

void hmac(char **hex, char *key, size_t key_len, char *data, size_t data_len) {
    unsigned int i = 0, l = 0;
    char k_ipad[65], k_opad[65];
    char *t_key = NULL, *digest1 = NULL, *digest2 = NULL;
    SHA1_CTX context;

    if (key_len > 64) {
        SHA1_CTX t_context;

        SHA1Init(&t_context);
        SHA1Update(&t_context, key, key_len);
        SHA1Final(&t_key, &t_context);

        key = t_key;
        key_len = 20;
    }

    memset(k_ipad, '\0', 65);
    memset(k_opad, '\0', 65);
    memcpy(k_ipad, key, key_len);
    memcpy(k_opad, key, key_len);

    for (i = 0, l = 64; i < l; i++) {
        k_ipad[i] ^= 0x36;
        k_opad[i] ^= 0x5C;
    }

    SHA1Init(&context);

    SHA1Update(&context, k_ipad, 64);
    SHA1Update(&context, data, data_len);
    SHA1Final(&digest1, &context);

    SHA1Init(&context);

    SHA1Update(&context, k_opad, 64);
    SHA1Update(&context, digest1, 20);

    SHA1Final(&digest2, &context);

    digest2hex(&*hex, 20, digest2);

    nessemble_free(t_key);
    nessemble_free(digest1);
    nessemble_free(digest2);
}
