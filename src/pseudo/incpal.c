#include <stdlib.h>
#include "../nessemble.h"
#include "../png.h"

void pseudo_incpal(char *string) {
    int color_mode = 3, color = 0, last_color = -1;
    unsigned int x = 0, y = 0, color_count = 0;
    char *path = NULL;
    unsigned char *rgb = NULL;
    struct png_data png = { 0, 0, NULL };

    if (get_fullpath(&path, string) != 0) {
        yyerror(_("Could not get full path of %s"), string);
        goto cleanup;
    }

    png = read_png(path);

    if (error_exists() != RETURN_OK) {
        goto cleanup;
    }

    for (y = 0; y < png.height && color_count < 4; y++) {
        for (x = 0; x < png.width && color_count < 4; x++) {
            rgb = &(png.data[(x * color_mode) + (y * (png.width * color_mode))]);
            color = match_color(rgb);

            if (color != last_color) {
                write_byte(color);
                last_color = color;
                color_count++;
            }
        }
    }

cleanup:
    nessemble_free(path);
    free_png(png);
}
