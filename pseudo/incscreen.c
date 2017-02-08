#include <stdlib.h>
#include "../nessemble.h"
#include "../png.h"

#define SCREEN_WIDTH  256
#define SCREEN_HEIGHT 240

void pseudo_incscreen(char *string, char *type) {
    int color_mode = 0, color = 0;
    unsigned int x = 0, y = 0, pixel_index = 0;
    unsigned int pixels[SCREEN_WIDTH * SCREEN_HEIGHT];
    char *path = NULL;
    struct png_data png = { NULL, 0, 0, {  }, 0, 0, 0, NULL, 0 };

    if (get_fullpath(&path, string) != 0) {
        yyerror("Could not get full path of %s", string);
        goto cleanup;
    }

    png = read_png(path);

    if (error_exists() != RETURN_OK) {
        goto cleanup;
    }

    // validate width/height
    if (png.width != SCREEN_WIDTH) {
        yyerror("Incorrect screen width (must be %d)", SCREEN_WIDTH);
    }

    if (png.height != SCREEN_HEIGHT) {
        yyerror("Incorrect screen height (must be %d)", SCREEN_HEIGHT);
    }

    color_mode = png_color_mode(png.color_type);

    // get pixels
    for (y = 0; y < png.height; y++) {
        png_byte *row = png.row_pointers[y];

        for (x = 0; x < png.width; x++) {
            png_byte *rgb = &(row[x * color_mode]);
            color = match_color(rgb, color_mode);

            pixels[pixel_index++] = (unsigned int)color;
        }
    }

cleanup:
    if (path) {
        free(path);
    }

    free_png(png);
}
