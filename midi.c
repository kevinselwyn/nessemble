#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nessemble.h"
#include "midi.h"

char *notes[12] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };

static unsigned int parse_events(struct midi_track *track, unsigned int /*@unused@*/quarter) {
    unsigned int rc = RETURN_OK;
    unsigned int i = 0, j = 0, k = 0, length = 0;
    unsigned int offset = 0, status = 0, tempo = 0, byte[10];
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

        status = (unsigned int)data[i] & 0xFF;

        if (status == 0xFF) {
            byte[1] = (unsigned int)data[i+1] & 0xFF;

            if (byte[1] == MIDI_META_EVENT_TRACK_COPYRIGHT) {
                //fprintf(stderr, "MIDI_META_EVENT_TRACK_COPYRIGHT\n");
                //event_count++;
                //byte[2] = (unsigned int)data[i+2] & 0xFF;
                //i += 3 + byte[2];
                //continue;
            }

            if (byte[1] == MIDI_META_EVENT_TRACK_NAME) {
                fprintf(stderr, "MIDI_META_EVENT_TRACK_NAME\n");

                byte[2] = (unsigned int)data[i+2] & 0xFF;

                fprintf(stderr, "  Name: ");

                for (j = 0, k = byte[2]; j < k; j++) {
                    fprintf(stderr, "%c", (char)data[i+j+3]);
                }

                fprintf(stderr, "\n");

                event_count++;
                i += 3 + byte[2];
                continue;
            }

            if (byte[1] == MIDI_META_EVENT_DEVICE_NAME) {
                fprintf(stderr, "MIDI_META_EVENT_DEVICE_NAME\n");

                byte[2] = (unsigned int)data[i+2] & 0xFF;

                fprintf(stderr, "  Name: ");

                for (j = 0, k = byte[2]; j < k; j++) {
                    fprintf(stderr, "%c", (char)data[i+j+3]);
                }

                fprintf(stderr, "\n");

                event_count++;
                i += 3 + byte[2];
                continue;
            }

            if (byte[1] == MIDI_META_EVENT_TEMPO) {
                fprintf(stderr, "0xFF 0x%02X - MIDI_META_EVENT_TEMPO\n", byte[1]);

                byte[3] = (unsigned int)data[i+3] & 0xFF;
                byte[4] = (unsigned int)data[i+4] & 0xFF;
                byte[5] = (unsigned int)data[i+5] & 0xFF;

                tempo = 0;
                tempo |= byte[3] << 16;
                tempo |= byte[4] << 8;
                tempo |= byte[5];

                fprintf(stderr, "  Tempo: %u\n", tempo);

                event_count++;
                i += 6;
                continue;
            }

            if (byte[1] == MIDI_META_EVENT_SMPTE) {
                fprintf(stderr, "0xFF 0x%02X - MIDI_META_EVENT_SMPTE\n", byte[1]);

                byte[3] = (unsigned int)data[i+3] & 0xFF;
                fprintf(stderr, "  Hours:     %u\n", byte[3]);

                byte[4] = (unsigned int)data[i+4] & 0xFF;
                fprintf(stderr, "  Minutes:   %u\n", byte[4]);

                byte[5] = (unsigned int)data[i+5] & 0xFF;
                fprintf(stderr, "  Seconds:   %u\n", byte[5]);

                byte[6] = (unsigned int)data[i+6] & 0xFF;
                fprintf(stderr, "  Frames:    %u\n", byte[6]);

                byte[7] = (unsigned int)data[i+7] & 0xFF;
                fprintf(stderr, "  Subframes: %u\n", byte[7]);

                event_count++;
                i += 8;
                continue;
            }

            if (byte[1] == MIDI_META_EVENT_TIME_SIGNATURE) {
                fprintf(stderr, "0xFF 0x%02X - MIDI_META_EVENT_TIME_SIGNATURE\n", byte[1]);

                byte[3] = (unsigned int)data[i+3] & 0xFF;
                fprintf(stderr, "  Numerator:   %u\n", byte[3]);

                byte[4] = (unsigned int)data[i+4] & 0xFF;
                fprintf(stderr, "  Denominator: %u\n", byte[4]);

                byte[5] = (unsigned int)data[i+5] & 0xFF;
                fprintf(stderr, "  Clocks:      %u\n", byte[5]);

                byte[6] = (unsigned int)data[i+6] & 0xFF;
                fprintf(stderr, "  32nds:       %u\n", byte[6]);

                event_count++;
                i += 7;
                continue;
            }

            if (byte[1] == MIDI_META_EVENT_KEY_SIGNATURE) {
                fprintf(stderr, "0xFF 0x%02X - MIDI_META_EVENT_KEY_SIGNATURE\n", byte[1]);

                byte[3] = (unsigned int)data[i+3] & 0xFF;
                fprintf(stderr, "  Sharps/Flats: %u\n", byte[3]);

                byte[4] = (unsigned int)data[i+4] & 0xFF;
                fprintf(stderr, "  Minor:        %u\n", byte[4]);

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

        if ((status >> 4) == 0x08) {
            fprintf(stderr, "MIDI_EVENT_NOTE_OFF\n");

            byte[1] = (unsigned int)data[i+1] & 0xFF;
            byte[2] = (unsigned int)data[i+2] & 0xFF;

            fprintf(stderr, "  Note Off Channel %u\n", status & 0x0F);
            fprintf(stderr, "  Note Off Key %u\n", byte[1]);
            fprintf(stderr, "  Note Off Velocity %u\n", byte[2]);

            event_count++;
            i += 3;
            continue;
        }

        if ((status >> 4) == 0x09) {
            fprintf(stderr, "MIDI_EVENT_NOTE_ON\n");

            byte[1] = (unsigned int)data[i+1] & 0xFF;
            byte[2] = (unsigned int)data[i+2] & 0xFF;

            fprintf(stderr, "  Note On Channel %u\n", status & 0x0F);
            fprintf(stderr, "  Note On Key %u\n", byte[1]);
            fprintf(stderr, "  Note On Velocity %u\n", byte[2]);

            event_count++;
            i += 3;
            continue;
        }

        if ((status >> 4) == 0x0B) {
            fprintf(stderr, "MIDI_EVENT_PRESSURE\n");

            byte[1] = (unsigned int)data[i+1] & 0xFF;
            byte[2] = (unsigned int)data[i+2] & 0xFF;

            fprintf(stderr, "  Pressure Channel %u\n", status & 0x0F);
            fprintf(stderr, "  Pressure Value 1 %u\n", byte[1]);
            fprintf(stderr, "  Pressure Value 2 %u\n", byte[2]);

            event_count++;
            i += 2;
            continue;
        }

        if ((status >> 4) == 0x0C) {
            fprintf(stderr, "MIDI_EVENT_INSTRUMENT\n");

            byte[1] = (unsigned int)data[i+1] & 0xFF;

            fprintf(stderr, "  Instrument Channel %u\n", status & 0x0F);
            fprintf(stderr, "  Instrument Number %u\n", byte[1]);

            event_count++;
            i += 2;
            continue;
        }

        if ((status & 0x80) == 0) {
            continue;
        }

        fprintf(stderr, "Unknown event 0x%02X\n", status);
        break;
    }

    events = (struct midi_event *)nessemble_malloc(sizeof(struct midi_event) * event_count);

    // pass 2

    track->events = events;
    track->event_count = event_count;

    return rc;
}

struct midi_data read_midi(char *filename) {
    unsigned int track = 0;
    size_t length = 0;
	FILE *file = NULL;
	struct midi_data midi;

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

    midi.tracks = (struct midi_track *)nessemble_malloc(sizeof(struct midi_track) * midi.track_count);

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

        midi.tracks[track].data = (char *)nessemble_malloc(sizeof(char) * (midi.tracks[track].length + 1));

        if (fread(midi.tracks[track].data, 1, (size_t)midi.tracks[track].length, file) != midi.tracks[track].length) {
            continue;
        }

        parse_events(&midi.tracks[track], midi.quarter);

        fprintf(stderr, "\n");
    }

cleanup:
    return midi;
}
