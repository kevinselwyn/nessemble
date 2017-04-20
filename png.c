#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "nessemble.h"
#include "png.h"

#define STB_IMAGE_IMPLEMENTATION
#include "third-party/stb/stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "third-party/stb/stb_image_write.h"

static int colors_1bps[COLOR_COUNT_2BIT] = { 0x00, 0x55, 0xAA, 0xFF };
static int colors_full[COLOR_COUNT_FULL] = {
    0x7C7C7C, 0x0000FC, 0x0000BC, 0x4428BC, 0x940084, 0xA80020, 0xA81000, 0x881400,
    0x503000, 0x007800, 0x006800, 0x005800, 0x004058, 0x000000, 0x000000, 0x000000,
    0xBCBCBC, 0x0078F8, 0x0058F8, 0x6844FC, 0xD800CC, 0xE40058, 0xF83800, 0xE45C10,
    0xAC7C00, 0x00B800, 0x00A800, 0x00A844, 0x008888, 0x000000, 0x000000, 0x000000,
    0xF8F8F8, 0x3CBCFC, 0x6888FC, 0x9878F8, 0xF878F8, 0xF85898, 0xF87858, 0xFCA044,
    0xF8B800, 0xB8F818, 0x58D854, 0x58F898, 0x00E8D8, 0x787878, 0x000000, 0x000000,
    0xFCFCFC, 0xA4E4FC, 0xB8B8F8, 0xD8B8F8, 0xF8B8F8, 0xF8A4C0, 0xF0D0B0, 0xFCE0A8,
    0xF8D878, 0xD8F878, 0xB8F8B8, 0xB8F8D8, 0x00FCFC, 0xF8D8F8, 0x000000, 0x000000
};

void free_png(struct png_data png) {
    nessemble_free(png.data);
}

struct png_data read_png(char *filename) {
    int width = 0, height = 0, n = 0;
    unsigned char *data = NULL;
    struct png_data png = { 0, 0, NULL };

    if ((data = stbi_load(filename, &width, &height, &n, 3)) == NULL) {
        error(_("Could not load PNG"));
        return png;
    }

    png.data = data;
    png.width = width;
    png.height = height;

    return png;
}

unsigned int write_png(unsigned int *pixels, unsigned int width, unsigned int height, char *filename) {
    unsigned int rc = RETURN_OK, i = 0, l = 0, index = 0;
    unsigned char *data = NULL;

    data = (unsigned char *)nessemble_malloc(sizeof(unsigned char) * ((width * height) * 3));

    for (i = 0, l = (width * height); i < l; i++) {
        data[index++] = colors_1bps[pixels[i]];
        data[index++] = colors_1bps[pixels[i]];
        data[index++] = colors_1bps[pixels[i]];
    }

    if (stbi_write_png(filename, width, height, 3, data, width * 3) == FALSE) {
        rc = RETURN_EPERM;
    }

    return rc;
}

int get_color(unsigned char *rgb, int color_mode) {
    int avg = 0, diff = 256, color = 0, i = 0, l = 0;

    for (i = 0, l = color_mode; i < l; i++) {
        avg += rgb[i];
    }

    avg /= color_mode;

    for (i = 0, l = COLOR_COUNT_2BIT; i < l; i++) {
        if (abs(colors_1bps[i] - avg) < diff) {
            diff = abs(colors_1bps[i] - avg);
            color = i;
        }
    }

    return color;
}

int match_color(unsigned char *rgb) {
    int r1 = 0, g1 = 0, b1 = 0, r2 = 0, g2 = 0, b2 = 0;
    int diff = 0xFFFFFF, next_diff = 0xFFFFFF, color = 0, i = 0, l = 0;

    for (i = 0, l = COLOR_COUNT_FULL; i < l; i++) {
        r1 = rgb[0];
        g1 = rgb[1];
        b1 = rgb[2];

        r2 = (colors_full[i] >> 16) & 0xFF;
        g2 = (colors_full[i] >> 8) & 0xFF;
        b2 = colors_full[i] & 0xFF;

        next_diff = sqrt(pow(r2 - r1, 2) + pow(g2 - g1, 2) + pow(b2 - b1, 2));

        if (next_diff < diff) {
            diff = next_diff;
            color = i;
        }
    }

    if (color == 0x0D) {
        color = 0x0F;
    }

    return color;
}
