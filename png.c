#include <stdlib.h>
#include <string.h>
#include <png.h>
#include "nessemble.h"

#define COLOR_COUNT 4

int colors[COLOR_COUNT] = { 0x00, 0x55, 0xAA, 0xFF };

struct png_data {
	int *bits, width, height;
    char *header[8];
	png_byte color_type, bit_depth;
	png_structp png_ptr;
	png_infop info_ptr;
	png_bytep *row_pointers;
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
	FILE *file = NULL;
	struct png_data png = { NULL, 0, 0, {  }, 0, 0, 0, NULL, 0 };

	file = fopen(filename, "rb");

	if (!file) {
		yyerror("Could not open %s\n", filename);
        goto cleanup;
	}

	if (fread(png.header, 1, 8, file) != 8) {
		yyerror("Could not read %s\n", filename);
        goto cleanup;
	}

	if (png_sig_cmp((png_bytep)png.header, 0, 8)) {
		yyerror("%s is not a PNG\n", filename);
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

	png_read_update_info(png.png_ptr, png.info_ptr);

	if (setjmp(png_jmpbuf(png.png_ptr))) {
		yyerror("Could not read PNG\n");
        goto cleanup;
	}

	png.row_pointers = (png_bytep*) malloc(sizeof(png_bytep) * png.height);

	for (y = 0; y < png.height; y++) {
		png.row_pointers[y] = (png_byte*) malloc(png_get_rowbytes(png.png_ptr, png.info_ptr));
	}

	png_read_image(png.png_ptr, png.row_pointers);

cleanup:
	fclose(file);

	return png;
}

int get_color(png_byte *rgb, int color_mode) {
    int avg = 0, diff = 256, color = 0, i = 0, l = 0;

    for (i = 0, l = color_mode; i < l; i++) {
        avg += rgb[i];
    }

    avg /= color_mode;

    for (i = 0, l = COLOR_COUNT; i < l; i++) {
        if (abs(colors[i] - avg) < diff) {
            diff = abs(colors[i] - avg);
            color = i;
        }
    }

    return color;
}

void pseudo_incpng(char *string, int offset, int limit) {
    int color_mode = 0, color = 0, byte = 0, tile_index = -1;
    int h = 0, w = 0, x = 0, y = 0;
    size_t path_length = 0, string_length = 0;
    char *path = NULL;
    struct png_data png = { NULL, 0, 0, {  }, 0, 0, 0, NULL, 0 };

    string_length = strlen(string);
    path_length = strlen(cwd_path) + string_length - 1;
    path = (char *)malloc(sizeof(char) * (path_length + 1));

    if (!path) {
        yyerror("Memory error");
        goto cleanup;
    }

    strcpy(path, cwd_path);
    strcat(path, "/");
    strncat(path, string + 1, string_length - 2);

    png = read_png(path);
    color_mode = png_color_mode(png.color_type);

    for (h = 0; h < png.height; h += 8) {
        for (w = 0; w < png.width; w += 8) {
            tile_index++;

            if (tile_index < offset) {
                continue;
            }

            if (limit != -1 && tile_index - offset >= limit) {
                continue;
            }

            for (y = h; y < h + 8; y++) {
                png_byte *row = png.row_pointers[y];
                byte = 0;

                for (x = w; x < w + 8; x++) {
                    png_byte *rgb = &(row[x * color_mode]);
                    color = get_color(rgb, color_mode);

                    byte |= (color & 0x01) << (7 - (x % 8));
                }

                write_byte(byte);
            }

            for (y = h; y < h + 8; y++) {
                png_byte *row = png.row_pointers[y];
                byte = 0;

                for (x = w; x < w + 8; x++) {
                    png_byte *rgb = &(row[x * color_mode]);
                    color = get_color(rgb, color_mode);

                    byte |= ((color >> 1) & 0x01) << (7 - (x % 8));
                }

                write_byte(byte);
            }
        }
    }

cleanup:
    if (path) {
        free(path);
    }

    free_png(png);
}
