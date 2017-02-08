#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nessemble.h"
#include "midi.h"

char *notes[12] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };

static unsigned int parse_events(struct midi_track *track, unsigned int quarter) {
    unsigned int rc = RETURN_OK;
    unsigned int i = 0, length = 0;
    unsigned int offset = 0, byte[4];
    unsigned int event_count = 0;
    char *data = NULL;
    struct midi_event *events = NULL;

    data = track->data;
    length = track->length;

    // pass 1
    while (i < length) {
        offset = (unsigned int)data[i++] & 0xFF;

        if (i >= length) {
            break;
        }

        byte[0] = (unsigned int)data[i] & 0xFF;

        if (byte[0] == 0xFF) {
            byte[1] = (unsigned int)data[i+1] & 0xFF;

            if (byte[1] == MIDI_META_EVENT_TRACK_COPYRIGHT) {
                fprintf(stderr, "MIDI_META_EVENT_TRACK_COPYRIGHT\n");
                event_count++;
                byte[2] = (unsigned int)data[i+2] & 0xFF;
                i += 3 + byte[2];
                continue;
            }

            if (byte[1] == MIDI_META_EVENT_TRACK_NAME) {
                fprintf(stderr, "MIDI_META_EVENT_TRACK_NAME\n");
                event_count++;
                byte[2] = (unsigned int)data[i+2] & 0xFF;
                i += 3 + byte[2];
                continue;
            }

            if (byte[1] == MIDI_META_EVENT_DEVICE_NAME) {
                fprintf(stderr, "MIDI_META_EVENT_DEVICE_NAME\n");
                event_count++;
                byte[2] = (unsigned int)data[i+2] & 0xFF;
                i += 3 + byte[2];
                continue;
            }

            if (byte[1] == MIDI_META_EVENT_TEMPO) {
                fprintf(stderr, "MIDI_META_EVENT_TEMPO\n");
                event_count++;
                i += 6;
                continue;
            }

            if (byte[1] == MIDI_META_EVENT_SMPTE) {
                fprintf(stderr, "MIDI_META_EVENT_SMPTE\n");
                event_count++;
                i += 8;
                continue;
            }

            if (byte[1] == MIDI_META_EVENT_TIME_SIGNATURE) {
                fprintf(stderr, "MIDI_META_EVENT_TIME_SIGNATURE\n");
                event_count++;
                i += 7;
                continue;
            }

            if (byte[1] == MIDI_META_EVENT_KEY_SIGNATURE) {
                fprintf(stderr, "MIDI_META_EVENT_KEY_SIGNATURE\n");
                event_count++;
                i += 5;
                continue;
            }

            if (byte[1] == MIDI_META_EVENT_END) {
                fprintf(stderr, "MIDI_META_EVENT_END\n");
                event_count++;
                i += 3;
                break;
            }

            fprintf(stderr, "Unknown event 0xFF 0x%02X\n", byte[1]);
            break;
        }

        if ((byte[0] >> 4) == 0x08) {
            fprintf(stderr, "MIDI_EVENT_NOTE_OFF\n");
            event_count++;
            i += 3;
            continue;
        }

        if ((byte[0] >> 4) == 0x09) {
            fprintf(stderr, "MIDI_EVENT_NOTE_ON\n");
            event_count++;
            i += 3;
            continue;
        }

        if ((byte[0] >> 4) == 0x0C) {
            fprintf(stderr, "MIDI_EVENT_INSTRUMENT\n");
            event_count++;
            i += 2;
            continue;
        }

        fprintf(stderr, "Unknown event 0x%02X\n", byte[0]);
        break;
    }

    events = (struct midi_event *)malloc(sizeof(struct midi_event) * event_count);

    if (!events) {
        goto cleanup;
    }

    // pass 2

cleanup:
    track->events = events;
    track->event_count = event_count;

    return rc;
}

struct midi_data read_midi(char *filename) {
    unsigned int track = 0;
    size_t length = 0;
	FILE *file = NULL;
	struct midi_data midi = { { }, 0, 0, 0, 0, NULL };

	file = fopen(filename, "rb");

    length = strlen(cwd_path) + 1;

	if (!file) {
		yyerror("Could not open `%s`", filename+length);
        goto cleanup;
	}

	if (fread(midi.header, 1, 4, file) != 4) {
		yyerror("Could not read `%s`", filename+length);
        goto cleanup;
	}

    fseek(file, 4, SEEK_SET);

    midi.header_length = fgetu32_big(file);
    midi.format = fgetu16_big(file);
    midi.track_count = fgetu16_big(file);
    midi.quarter = fgetu16_big(file);

    if (strncmp((const char *)midi.header, "MThd", 4) != 0) {
        yyerror("`%s` is not a MIDI file", filename+length);
        goto cleanup;
    }

    if (midi.format != MIDI_FORMAT_1) {
        yyerror("Invalid MIDI format");
        goto cleanup;
    }

    if (midi.track_count <= 0) {
        yyerror("Could not find any MIDI tracks");
        goto cleanup;
    }

    if (midi.quarter <= 0) {
        yyerror("Invalid tempo");
        goto cleanup;
    }

    fprintf(stderr, "Tracks:  %d\n", midi.track_count);
    fprintf(stderr, "Quarter: %d\n\n", midi.quarter);

    midi.tracks = (struct midi_track *)malloc(sizeof(struct midi_track) * midi.track_count);

    for (track = 0; track < midi.track_count; track++) {
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

        parse_events(&midi.tracks[track], midi.quarter);

        fprintf(stderr, "\n");
    }

cleanup:
    return midi;
}
