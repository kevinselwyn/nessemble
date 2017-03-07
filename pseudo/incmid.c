#include <stdlib.h>
#include "../nessemble.h"
#include "../midi.h"

/**
 * .incmid pseudo instruction
 * @param {char *} string - Filename of MIDI file
 */
void pseudo_incmid(char *string) {
    char *path = NULL;
    struct midi_data midi;

    if (get_fullpath(&path, string) != 0) {
        yyerror("Could not get full path of %s", string);
        goto cleanup;
    }

    midi = read_midi(path);

    if (error_exists() != RETURN_OK) {
        goto cleanup;
    }

cleanup:
    nessemble_free(path);
}
