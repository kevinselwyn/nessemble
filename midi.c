#include <stdio.h>
#include <string.h>
#include "nessemble.h"
#include "midi.h"

struct midi_event *parse_events(char *data, unsigned int length, unsigned int quarter) {
    unsigned int i = 0;
    unsigned int offset = 0, beat = 0, byte[3];
    struct midi_event *events = NULL;

    events = (struct midi_event *)malloc(sizeof(struct midi_event) * 10);

    if (!events) {
        goto cleanup;
    }

    beat = 0;

    while (i < length) {
        offset = (unsigned int)data[i] & 0xFF;
        beat = (beat + offset) % quarter;
        i++;

        if (i >= length) {
            break;
        }

        byte[0] = (unsigned int)data[i] & 0xFF;

        if (byte[0] == 0xFF) {
            byte[1] = (unsigned int)data[i+1] & 0xFF;

            if (byte[1] == 0x03) {
                byte[2] = (unsigned int)data[i+2] & 0xFF;

                fprintf(stderr, "%d Track title\n", beat);

                i += 3 + byte[2];
                continue;
            }

            if (byte[1] == 0x2F) {
                fprintf(stderr, "%d Track end\n", beat);

                i += 3;
                continue;
            }
        }

        if ((byte[0] >> 4) == 0x8) {
            fprintf(stderr, "%d Note off\n", beat);

            i += 3;
            continue;
        }

        if ((byte[0] >> 4) == 0x9) {
            fprintf(stderr, "%d Note on\n", beat);

            i += 3;
            continue;
        }

        if ((byte[0] >> 4) == 0xC) {
            fprintf(stderr, "%d Channel instrument\n", beat);

            i += 2;
            continue;
        }

        fprintf(stderr, "Unknown even 0x%02X\n", byte[0]);
    }

cleanup:
    return events;
}

struct midi_data read_midi(char *filename) {
    unsigned int track = 0, index = 0;
    size_t length = 0;
	FILE *file = NULL;
	struct midi_data midi = { { }, 0, 0, 0, 0, NULL };

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
    midi.track_count = fgetu16_big(file);
    midi.quarter = fgetu16_big(file);

    if (strncmp((const char *)midi.header, "MThd", 4) != 0) {
        error("`%s` is not a MIDI file", filename+length);
        goto cleanup;
    }

    if (midi.format != MIDI_FORMAT_1) {
        error("Invalid MIDI format");
        goto cleanup;
    }

    if (midi.track_count <= 0) {
        error("Could not find any MIDI tracks");
        goto cleanup;
    }

    if (midi.quarter <= 0) {
        error("Invalid tempo");
        goto cleanup;
    }

    fprintf(stderr, "Tracks:  %d\n", midi.track_count);
    fprintf(stderr, "Quarter: %d\n", midi.quarter);

    midi.tracks = (struct midi_track *)malloc(sizeof(struct midi_track) * midi.track_count);

    for (track = 0; track < midi.track_count; track++) {
        index = (unsigned int)ftell(file);

        if (fread(midi.tracks[track].header, 1, 4, file) != 4) {
    		error("Could not read track %d in `%s`", track, filename+length);
            continue;
    	}

        if (strncmp((const char *)midi.tracks[track].header, "MTrk", 4) != 0) {
            error("Invalid track %d in `%s`", track, filename+length);
            continue;
        }

        midi.tracks[track].length = fgetu32_big(file);

        if (midi.tracks[track].length == 0) {
            continue;
        }

        midi.tracks[track].data = (char *)malloc(sizeof(char) * (midi.tracks[track].length + 1));

        if (!midi.tracks[track].data) {
            fprintf(stderr, "Memory error\n");
            goto cleanup;
        }

        if (fread(midi.tracks[track].data, 1, midi.tracks[track].length, file) != midi.tracks[track].length) {
            continue;
        }

        midi.tracks[track].events = parse_events(midi.tracks[track].data, midi.tracks[track].length, midi.quarter);
    }

cleanup:
    return midi;
}
