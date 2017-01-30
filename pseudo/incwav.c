#include <stdlib.h>
#include "../nessemble.h"
#include "../wav.h"

void pseudo_incwav(char *string, int amplitude) {
    int x = 0, y = 0;
    int subsample = 99, oversample = 100;
    long filelen = 0;
    char *path = NULL;
    wave_src wav = { { 0, 0, 0, 0, 0, 0 } , NULL, 0, 0 };

    if (get_fullpath(&path, string) != 0) {
        yyerror("Could not get full path of %s", string);
        goto cleanup;
    }

    if (!path) {
        yyerror("Could not get full path of %s", string);
        goto cleanup;
    }

    if (open_wav(&wav, path) != 0) {
        goto cleanup;
    }

    if (wav.fmt.channels != 1) {
        yyerror("%s is not mono", string);
        goto cleanup;
    }

    if (error_exists() == TRUE) {
        goto cleanup;
    }

    if (amplitude < 2) {
        amplitude = 2;
    }

    if (amplitude > 40) {
        amplitude = 40;
    }

    while (wav.chunk_left > 0) {
        unsigned int i = 0, code = 0;

        for (i = 0; i < 8; i++) {
            while (subsample < 100) {
                x = (wav_sample(&wav) * amplitude + 16384) >> 15;
                filelen--;
                subsample += oversample;
            }

            subsample -= 100;

            if (x >= y) {
                y++;

                if (y > 31) {
                    y = 31;
                }

                code |= 1 << i;
            } else {
                y--;

                if (y < -32) {
                    y = -32;
                }
            }
        }

        write_byte(code);
    }

cleanup:
    if (path) {
        free(path);
    }

    close_wav(&wav);
}
