#include <stdlib.h>
#include "../nessemble.h"
#include "../png.h"

void pseudo_incpng(char *string, int offset, int limit) {
    int color_mode = 3, color = 0, byte = 0, tile_index = -1;
    unsigned int h = 0, w = 0, x = 0, y = 0;
    char *path = NULL;
    unsigned char *rgb = NULL;
    struct png_data png;

    if (get_fullpath(&path, string) != 0) {
        yyerror("Could not get full path of %s", string);
        goto cleanup;
    }

    png = read_png(path);

    if (error_exists() != RETURN_OK) {
        goto cleanup;
    }

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
                byte = 0;

                for (x = w; x < w + 8; x++) {
                    rgb = &(png.data[(x * color_mode) + (y * (png.width * color_mode))]);
                    color = get_color(rgb, color_mode);

                    byte |= (color & 0x01) << (7 - (x % 8));
                }

                write_byte(byte);
            }

            for (y = h; y < h + 8; y++) {
                byte = 0;

                for (x = w; x < w + 8; x++) {
                    rgb = &(png.data[(x * color_mode) + (y * (png.width * color_mode))]);
                    color = get_color(rgb, color_mode);

                    byte |= ((color >> 1) & 0x01) << (7 - (x % 8));
                }

                write_byte(byte);
            }
        }
    }

cleanup:
    nessemble_free(path);
    free_png(png);
}
