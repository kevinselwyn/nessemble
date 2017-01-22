#include <stdlib.h>
#include "../nessemble.h"
#include "../png.h"

void pseudo_incpal(char *string) {
    int color_mode = 0, color = 0, last_color = -1, x = 0;
    char *path = NULL;
    struct png_data png = { NULL, 0, 0, {  }, 0, 0, 0, NULL, 0 };

    if (get_fullpath(&path, string) != 0) {
        yyerror("Could not get full path of %s", string);
        goto cleanup;
    }

    png = read_png(path);
    color_mode = png_color_mode(png.color_type);

    png_byte *row = png.row_pointers[0];

    for (x = 0; x < png.width; x++) {
        png_byte *rgb = &(row[x * color_mode]);
        color = match_color(rgb, color_mode);

        if (color != last_color) {
            write_byte(color);
            last_color = color;
        }
    }

cleanup:
    if (path) {
        free(path);
    }

    free_png(png);
}
