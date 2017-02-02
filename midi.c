#include <stdio.h>
#include <string.h>
#include "nessemble.h"
#include "midi.h"

struct midi_data read_midi(char *filename) {
    size_t length = 0;
	FILE *file = NULL;
	struct midi_data midi = { { }, 0, 0, 0, 0 };

	file = fopen(filename, "rb");

    length = strlen(cwd_path) + 1;

	if (!file) {
		error("Could not open `%s`", filename+length);
        goto cleanup;
	}

	if (fread(midi.header, 1, 4, file) != 4) {
		error("Could not read `%s`", filename+length);
        goto cleanup;
	}

    fseek(file, 4, SEEK_SET);

    midi.header_length = fgetu32_big(file);
    midi.format = fgetu16_big(file);
    midi.tracks = fgetu16_big(file);
    midi.quarter = fgetu16_big(file);

    if (strncmp((const char *)midi.header, "MThd", 4) != 0) {
        error("`%s` is not a MIDI file", filename+length);
        goto cleanup;
    }

    if (midi.format != MIDI_FORMAT_1) {
        error("Invalid MIDI format");
        goto cleanup;
    }

    if (midi.tracks <= 0) {
        error("Could not find any MIDI tracks");
        goto cleanup;
    }

    if (midi.quarter <= 0) {
        error("Invalid tempo");
        goto cleanup;
    }

    fprintf(stderr, "Tracks:  %d\n", midi.tracks);
    fprintf(stderr, "Quarter: %d\n", midi.quarter);

    

cleanup:
    return midi;
}
