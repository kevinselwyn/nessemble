#include <stdlib.h>
#include "../nessemble.h"
#include "../png.h"

void pseudo_incpal(char *string) {
    int color_mode = 3, color = 0, last_color = -1;
    unsigned int x = 0;
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

    for (x = 0; x < png.width; x++) {
        rgb = &(png.data[x * color_mode]);
        color = match_color(rgb, color_mode);

        if (color != last_color) {
            write_byte(color);
            last_color = color;
        }
    }

cleanup:
    nessemble_free(path);
    free_png(png);
}
