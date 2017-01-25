#include <stdlib.h>
#include "../nessemble.h"
#include "../png.h"

void pseudo_incscreen(char *string, char *type) {
    int color_mode = 0, color = 0, byte = 0, tile_index = -1;
    int h = 0, w = 0, x = 0, y = 0;
    int i = 0, j = 0, k = 0, l = 0;
    char *path = NULL;
    struct png_data png = { NULL, 0, 0, {  }, 0, 0, 0, NULL, 0 };

    if (get_fullpath(&path, string) != 0) {
        yyerror("Could not get full path of %s", string);
        goto cleanup;
    }

    png = read_png(path);
    color_mode = png_color_mode(png.color_type);

    // validate width/height
    if (png.width != 256) {
        yyerror("Incorrect screen width (must be %d)", 256);
    }

    if (png.height != 240) {
        yyerror("Incorrect screen height (must be %d)", 240);
    }

    // validate colors
    for (i = 0, j = 16; i < j; i += 16) {
        for (k = 0, l = 16; k < l; k += 16) {
            for (y = i; y < i + 16; y++) {
                for (x = k; x < k + 16; x++) {
                    //fprintf(stderr, "%d %d\n", x, y);
                }
            }
        }
    }

cleanup:
    if (path) {
        free(path);
    }

    free_png(png);
}
