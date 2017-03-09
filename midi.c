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

    memset(byte, 0, 10);

    data = track->data;
    length = track->length;

    // pass 1
    i = 0;
    event_count = 0;

    while (i < length) {
        offset = (unsigned int)data[i++] & 0xFF;

        if (i >= length) {
            break;
        }

        status = (unsigned int)data[i] & 0xFF;

        if (status == 0xFF) {
            byte[1] = (unsigned int)data[i+1] & 0xFF;

            if (byte[1] == MIDI_META_EVENT_TRACK_COPYRIGHT) {
                //event_count++;
                //i += 3 + byte[2];
                //continue;
            }

            if (byte[1] == MIDI_META_EVENT_TRACK_NAME) {
                event_count++;
                i += 3 + byte[2];
                continue;
            }

            if (byte[1] == MIDI_META_EVENT_DEVICE_NAME) {
                event_count++;
                i += 3 + byte[2];
                continue;
            }

            if (byte[1] == MIDI_META_EVENT_TEMPO) {
                event_count++;
                i += 6;
                continue;
            }

            if (byte[1] == MIDI_META_EVENT_SMPTE) {
                event_count++;
                i += 8;
                continue;
            }

            if (byte[1] == MIDI_META_EVENT_TIME_SIGNATURE) {
                event_count++;
                i += 7;
                continue;
            }

            if (byte[1] == MIDI_META_EVENT_KEY_SIGNATURE) {
                event_count++;
                i += 5;
                continue;
            }

            if (byte[1] == MIDI_META_EVENT_END) {
                event_count++;
                i += 3;
                break;
            }

            break;
        }

        if ((status >> 4) == 0x08) {
            event_count++;
            i += 3;
            continue;
        }

        if ((status >> 4) == 0x09) {
            event_count++;
            i += 3;
            continue;
        }

        if ((status >> 4) == 0x0B) {
            event_count++;
            i += 2;
            continue;
        }

        if ((status >> 4) == 0x0C) {
            event_count++;
            i += 2;
            continue;
        }

        if ((status & 0x80) == 0) {
            continue;
        }

        break;
    }

    events = (struct midi_event *)malloc(sizeof(struct midi_event) * event_count);

    if (!events) {
        goto cleanup;
    }

    // pass 2
    i = 0;
    event_count = 0;

    while (i < length) {
        offset = (unsigned int)data[i++] & 0xFF;

        if (i >= length) {
            break;
        }

        status = (unsigned int)data[i] & 0xFF;

        if (status == 0xFF) {
            byte[1] = (unsigned int)data[i+1] & 0xFF;

            if (byte[1] == MIDI_META_EVENT_TRACK_COPYRIGHT) {
                ////fprintf(stderr, "MIDI_META_EVENT_TRACK_COPYRIGHT\n");
                //event_count++;
                //byte[2] = (unsigned int)data[i+2] & 0xFF;
                //i += 3 + byte[2];
                //continue;
            }

            if (byte[1] == MIDI_META_EVENT_TRACK_NAME) {
                byte[2] = (unsigned int)data[i+2] & 0xFF;

                events[event_count].type = MIDI_META_EVENT_TRACK_NAME;
                events[event_count].beat = offset;
                events[event_count].data = (unsigned int *)malloc(sizeof(unsigned int) * byte[2]);

                if (!events[event_count].data) {
                    continue;
                }

                for (j = 0, k = byte[2]; j < k; j++) {
                    events[event_count].data[events[event_count].data_index++] = (unsigned int)data[i+j+3] & 0xFF;
                }

                event_count++;
                i += 3 + byte[2];
                continue;
            }

            if (byte[1] == MIDI_META_EVENT_DEVICE_NAME) {
                byte[2] = (unsigned int)data[i+2] & 0xFF;

                events[event_count].type = MIDI_META_EVENT_DEVICE_NAME;
                events[event_count].beat = offset;
                events[event_count].data = (unsigned int *)malloc(sizeof(unsigned int) * byte[2]);

                if (!events[event_count].data) {
                    continue;
                }

                for (j = 0, k = byte[2]; j < k; j++) {
                    events[event_count].data[events[event_count].data_index++] = (unsigned int)data[i+j+3] & 0xFF;
                }

                event_count++;
                i += 3 + byte[2];
                continue;
            }

            if (byte[1] == MIDI_META_EVENT_TEMPO) {
                events[event_count].type = MIDI_META_EVENT_TEMPO;
                events[event_count].beat = offset;
                events[event_count].data = (unsigned int *)malloc(sizeof(unsigned int) * 1);

                byte[3] = (unsigned int)data[i+3] & 0xFF;
                byte[4] = (unsigned int)data[i+4] & 0xFF;
                byte[5] = (unsigned int)data[i+5] & 0xFF;

                tempo = 0;
                tempo |= byte[3] << 16;
                tempo |= byte[4] << 8;
                tempo |= byte[5];

                tempo = (unsigned int)((tempo / quarter) / 4);

                events[event_count].data[events[event_count].data_index++] = tempo;

                event_count++;
                i += 6;
                continue;
            }

            if (byte[1] == MIDI_META_EVENT_SMPTE) {
                events[event_count].type = MIDI_META_EVENT_SMPTE;
                events[event_count].beat = offset;
                events[event_count].data = (unsigned int *)malloc(sizeof(unsigned int) * 5);

                for (j = 0, k = 5; j < k; j++) {
                    events[event_count].data[events[event_count].data_index++] = (unsigned int)data[i+j+3] & 0xFF;
                }

                event_count++;
                i += 8;
                continue;
            }

            if (byte[1] == MIDI_META_EVENT_TIME_SIGNATURE) {
                //fprintf(stderr, "0xFF 0x%02X - MIDI_META_EVENT_TIME_SIGNATURE\n", byte[1]);

                byte[3] = (unsigned int)data[i+3] & 0xFF;
                //fprintf(stderr, "  Numerator:   %d\n", byte[3]);

                byte[4] = (unsigned int)data[i+4] & 0xFF;
                //fprintf(stderr, "  Denominator: %d\n", byte[4]);

                byte[5] = (unsigned int)data[i+5] & 0xFF;
                //fprintf(stderr, "  Clocks:      %d\n", byte[5]);

                byte[6] = (unsigned int)data[i+6] & 0xFF;
                //fprintf(stderr, "  32nds:       %d\n", byte[6]);

                event_count++;
                i += 7;
                continue;
            }

            if (byte[1] == MIDI_META_EVENT_KEY_SIGNATURE) {
                //fprintf(stderr, "0xFF 0x%02X - MIDI_META_EVENT_KEY_SIGNATURE\n", byte[1]);

                byte[3] = (unsigned int)data[i+3] & 0xFF;
                //fprintf(stderr, "  Sharps/Flats: %d\n", byte[3]);

                byte[4] = (unsigned int)data[i+4] & 0xFF;
                //fprintf(stderr, "  Minor:        %d\n", byte[4]);

                event_count++;
                i += 5;
                continue;
            }

            if (byte[1] == MIDI_META_EVENT_END) {
                //fprintf(stderr, "MIDI_META_EVENT_END\n");
                event_count++;
                i += 3;
                break;
            }

            //fprintf(stderr, "Unknown event 0xFF 0x%02X\n", byte[1]);
            break;
        }

        if ((status >> 4) == 0x08) {
            //fprintf(stderr, "MIDI_EVENT_NOTE_OFF\n");

            byte[1] = (unsigned int)data[i+1] & 0xFF;
            byte[2] = (unsigned int)data[i+2] & 0xFF;

            //fprintf(stderr, "  Note Off Channel %d\n", status & 0x0F);
            //fprintf(stderr, "  Note Off Key %d\n", byte[1]);
            //fprintf(stderr, "  Note Off Velocity %d\n", byte[2]);

            event_count++;
            i += 3;
            continue;
        }

        if ((status >> 4) == 0x09) {
            //fprintf(stderr, "MIDI_EVENT_NOTE_ON\n");

            byte[1] = (unsigned int)data[i+1] & 0xFF;
            byte[2] = (unsigned int)data[i+2] & 0xFF;

            //fprintf(stderr, "  Note On Channel %d\n", status & 0x0F);
            //fprintf(stderr, "  Note On Key %d\n", byte[1]);
            //fprintf(stderr, "  Note On Velocity %d\n", byte[2]);

            event_count++;
            i += 3;
            continue;
        }

        if ((status >> 4) == 0x0B) {
            //fprintf(stderr, "MIDI_EVENT_PRESSURE\n");

            byte[1] = (unsigned int)data[i+1] & 0xFF;
            byte[2] = (unsigned int)data[i+2] & 0xFF;

            //fprintf(stderr, "  Pressure Channel %d\n", status & 0x0F);
            //fprintf(stderr, "  Pressure Value 1 %d\n", byte[1]);
            //fprintf(stderr, "  Pressure Value 2 %d\n", byte[2]);

            event_count++;
            i += 2;
            continue;
        }

        if ((status >> 4) == 0x0C) {
            //fprintf(stderr, "MIDI_EVENT_INSTRUMENT\n");

            byte[1] = (unsigned int)data[i+1] & 0xFF;

            //fprintf(stderr, "  Instrument Channel %d\n", status & 0x0F);
            //fprintf(stderr, "  Instrument Number %d\n", byte[1]);

            event_count++;
            i += 2;
            continue;
        }

        if ((status & 0x80) == 0) {
            continue;
        }

        //fprintf(stderr, "Unknown event 0x%02X\n", status);
        break;
    }

cleanup:
    track->events = events;
    track->event_count = event_count;

    return rc;
}

struct midi_data read_midi(char *filename) {
    unsigned int track = 0, i = 0, j = 0, k = 0, l = 0;
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

    UNUSED(fseek(file, 4, SEEK_SET));

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

    fprintf(stderr, "Tracks:  %u\n", midi.track_count);
    fprintf(stderr, "Quarter: %u\n\n", midi.quarter);

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

        if (!midi.tracks[track].data) {
            //fprintf(stderr, "Memory error\n");
            goto cleanup;
        }

        if (fread(midi.tracks[track].data, 1, midi.tracks[track].length, file) != midi.tracks[track].length) {
            continue;
        }

        parse_events(&midi.tracks[track], midi.quarter);

        for (i = 0, j = midi.tracks[track].event_count; i < j; i++) {
            switch (midi.tracks[track].events[i].type) {
            case MIDI_META_EVENT_TRACK_NAME:
                fprintf(stderr, "Event: MIDI_META_EVENT_TRACK_NAME\n");
                fprintf(stderr, "  Beat: %d\n", midi.tracks[track].events[i].beat);
                fprintf(stderr, "  Name: ");

                if (midi.tracks[track].events[i].data_index) {
                    for (k = 0, l = midi.tracks[track].events[i].data_index; k < l; k++) {
                        fprintf(stderr, "%c", (char)midi.tracks[track].events[i].data[k]);
                    }
                }

                fprintf(stderr, "\n");
                break;
            case MIDI_META_EVENT_DEVICE_NAME:
                fprintf(stderr, "Event: MIDI_META_EVENT_DEVICE_NAME\n");
                fprintf(stderr, "  Beat: %d\n", midi.tracks[track].events[i].beat);
                fprintf(stderr, "  Name: ");

                if (midi.tracks[track].events[i].data_index) {
                    for (k = 0, l = midi.tracks[track].events[i].data_index; k < l; k++) {
                        fprintf(stderr, "%c", (char)midi.tracks[track].events[i].data[k]);
                    }
                }

                fprintf(stderr, "\n");
                break;
            case MIDI_META_EVENT_TEMPO:
                fprintf(stderr, "Event: MIDI_META_EVENT_TEMPO\n");
                fprintf(stderr, "  Beat: %d\n", midi.tracks[track].events[i].beat);
                fprintf(stderr, "  Tempo: %d\n", midi.tracks[track].events[i].data[0]);
                break;
            case MIDI_META_EVENT_SMPTE:
                fprintf(stderr, "Event: MIDI_META_EVENT_SMPTE\n");
                fprintf(stderr, "  Beat:      %d\n", midi.tracks[track].events[i].beat);
                fprintf(stderr, "  Hours:     %d\n", midi.tracks[track].events[i].data[0]);
                fprintf(stderr, "  Minutes:   %d\n", midi.tracks[track].events[i].data[1]);
                fprintf(stderr, "  Seconds:   %d\n", midi.tracks[track].events[i].data[2]);
                fprintf(stderr, "  Frames:    %d\n", midi.tracks[track].events[i].data[3]);
                fprintf(stderr, "  Subframes: %d\n", midi.tracks[track].events[i].data[4]);
                break;
            default:
                break;
            }
        }

        fprintf(stderr, "\n");
    }

cleanup:
    nessemble_fclose(file);

    return midi;
}
