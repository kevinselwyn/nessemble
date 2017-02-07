#include <stdlib.h>
#include <string.h>
#include <png.h>
#include "nessemble.h"
#include "png.h"

#define COLOR_COUNT_1BPS 0x04
#define COLOR_COUNT_FULL 0x40

int colors_1bps[COLOR_COUNT_1BPS] = { 0x00, 0x55, 0xAA, 0xFF };
int colors_full[COLOR_COUNT_FULL] = {
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
	int y = 0;

	for (y = 0; y < png.height; y++) {
        if (png.row_pointers[y]) {
            free(png.row_pointers[y]);
        }
	}

    if (png.row_pointers) {
        free(png.row_pointers);
    }

    if (png.png_ptr) {
        free(png.png_ptr);
    }

    if (png.info_ptr) {
        free(png.info_ptr);
    }
}

int png_color_mode(int color_type) {
	int color_mode = 0;

	switch (color_type) {
	case PNG_COLOR_TYPE_GRAY:
		color_mode = 1;
		break;
	default:
		color_mode = 3;
	}

	return color_mode;
}

struct png_data read_png(char *filename) {
    int y = 0;
    size_t length = 0;
	FILE *file = NULL;
	struct png_data png = { NULL, 0, 0, {  }, 0, 0, 0, NULL, 0 };

	file = fopen(filename, "rb");

    length = strlen(cwd_path) + 1;

	if (!file) {
		error("Could not open `%s`", filename+length);
        goto cleanup;
	}

	if (fread(png.header, 1, 8, file) != 8) {
		error("Could not read `%s`", filename+length);
        goto cleanup;
	}

	if (png_sig_cmp((unsigned char *)png.header, 0, 8)) {
		error("`%s` is not a PNG", filename+length);
        goto cleanup;
	}

	png.png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

	if (!png.png_ptr) {
		yyerror("png_create_read_struct failed\n");
        goto cleanup;
	}

	png.info_ptr = png_create_info_struct(png.png_ptr);

	if (!png.info_ptr) {
		yyerror("png_create_info_struct failed\n");
        goto cleanup;
	}

	if (setjmp(png_jmpbuf(png.png_ptr))) {
		yyerror("Error initializing read\n");
        goto cleanup;
	}

	png_init_io(png.png_ptr, file);
	png_set_sig_bytes(png.png_ptr, 8);
	png_read_info(png.png_ptr, png.info_ptr);

	png.width = png_get_image_width(png.png_ptr, png.info_ptr);
	png.height = png_get_image_height(png.png_ptr, png.info_ptr);
	png.color_type = png_get_color_type(png.png_ptr, png.info_ptr);
	png.bit_depth = png_get_bit_depth(png.png_ptr, png.info_ptr);

    if (png.bit_depth != 8) {
        yyerror("Image bit depth must be 8");
        goto cleanup;
    }

	png_read_update_info(png.png_ptr, png.info_ptr);

	if (setjmp(png_jmpbuf(png.png_ptr))) {
		yyerror("Could not read PNG\n");
        goto cleanup;
	}

	png.row_pointers = (unsigned char **) malloc(sizeof(unsigned char *) * png.height);

	for (y = 0; y < png.height; y++) {
		png.row_pointers[y] = (unsigned char *) malloc(png_get_rowbytes(png.png_ptr, png.info_ptr));
	}

	png_read_image(png.png_ptr, png.row_pointers);

cleanup:
    if (file) {
        fclose(file);
    }

	return png;
}

int get_color(unsigned char *rgb, int color_mode) {
    int avg = 0, diff = 256, color = 0, i = 0, l = 0;

    for (i = 0, l = color_mode; i < l; i++) {
        avg += rgb[i];
    }

    avg /= color_mode;

    for (i = 0, l = COLOR_COUNT_1BPS; i < l; i++) {
        if (abs(colors_1bps[i] - avg) < diff) {
            diff = abs(colors_1bps[i] - avg);
            color = i;
        }
    }

    return color;
}

int match_color(unsigned char *rgb, int color_mode) {
    int r1 = 0, g1 = 0, b1 = 0, r2 = 0, g2 = 0, b2 = 0;
    int diff = 0xFFFFFF, next_diff = 0xFFFFFF, color = 0, i = 0, l = 0;

    if (color_mode != 3) {
        fprintf(stderr, "Invalid color mode\n");
        goto cleanup;
    }

    for (i = 0, l = COLOR_COUNT_FULL; i < l; i++) {
        r1 = rgb[0];
        g1 = rgb[1];
        b1 = rgb[2];

        r2 = (colors_full[i] >> 16) & 0xFF;
        g2 = (colors_full[i] >> 8) & 0xFF;
        b2 = colors_full[i] & 0xFF;

        next_diff = root(power(r2 - r1, 2) + power(g2 - g1, 2) + power(b2 - b1, 2), 2);

        if (next_diff < diff) {
            diff = next_diff;
            color = i;
        }
    }

    if (color == 0x0D) {
        color = 0x0F;
    }

cleanup:
    return color;
}
