#include <stdlib.h>
#include "../nessemble.h"
#include "../png.h"

#define SCREEN_WIDTH  256
#define SCREEN_HEIGHT 240

void pseudo_incscreen(char *string, char *type) {
    int color_mode = 3, color = 0;
    unsigned int x = 0, y = 0, pixel_index = 0;
    unsigned int pixels[SCREEN_WIDTH * SCREEN_HEIGHT];
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

    // validate width/height
    if (png.width != SCREEN_WIDTH) {
        yyerror("Incorrect screen width (must be %d)", SCREEN_WIDTH);
    }

    if (png.height != SCREEN_HEIGHT) {
        yyerror("Incorrect screen height (must be %d)", SCREEN_HEIGHT);
    }

    // get pixels
    for (y = 0; y < png.height; y++) {
        for (x = 0; x < png.width; x++) {
            rgb = &(png.data[(x * color_mode) + (y * (png.width * color_mode))]);
            color = match_color(rgb);

            pixels[pixel_index++] = (unsigned int)color;
        }
    }

cleanup:
    nessemble_free(path);
}
