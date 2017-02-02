#include <stdio.h>
#include <string.h>
#include "nessemble.h"
#include "midi.h"

char *notes[12] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };

static unsigned int parse_events(struct midi_track *track, unsigned int quarter) {
    unsigned int rc = RETURN_OK;
    unsigned int i = 0, index = 0, length = 0;
    unsigned int offset = 0, beat = 0, byte[4];
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

            if (byte[1] == 0x03) {
                event_count++;
                byte[2] = (unsigned int)data[i+2] & 0xFF;
                i += 3 + byte[2];
                continue;
            }

            if (byte[1] == 0x58) {
                event_count++;
                i += 7;
                continue;
            }

            if (byte[1] == 0x51) {
                event_count++;
                i += 6;
                continue;
            }

            if (byte[1] == 0x2F) {
                event_count++;
                i += 3;
                break;
            }

            fprintf(stderr, "Unknown event 0xFF 0x%02X\n", byte[0]);
            rc = RETURN_EPERM;
            goto cleanup;
        }

        if ((byte[0] >> 4) == 0x8) {
            event_count++;
            i += 3;
            continue;
        }

        if ((byte[0] >> 4) == 0x9) {
            event_count++;
            i += 3;
            continue;
        }

        if ((byte[0] >> 4) == 0xC) {
            event_count++;
            i += 2;
            continue;
        }

        fprintf(stderr, "Unknown event 0x%02X\n", byte[0]);
        rc = RETURN_EPERM;
        goto cleanup;
    }

    events = (struct midi_event *)malloc(sizeof(struct midi_event) * event_count);

    if (!events) {
        goto cleanup;
    }

    i = 0;
    beat = 0;

    // pass 2
    while (i < length) {
        offset = (unsigned int)data[i++] & 0xFF;
        beat = (beat + offset) % quarter;

        if (i >= length) {
            break;
        }

        events[index].beat = (int)(((float)beat / (float)quarter) * 4.0);

        byte[0] = (unsigned int)data[i] & 0xFF;

        if (byte[0] == 0xFF) {
            byte[1] = (unsigned int)data[i+1] & 0xFF;

            if (byte[1] == 0x03) {
                byte[2] = (unsigned int)data[i+2] & 0xFF;

                events[index++].type = MIDI_EVENT_TITLE;

                i += 3 + byte[2];
                continue;
            }

            if (byte[1] == 0x58) {
                byte[2] = (unsigned int)data[i+3] & 0xFF;
                byte[3] = (unsigned int)data[i+4] & 0xFF;

                events[index].type = MIDI_EVENT_TIME_SIGNATURE;
                events[index].data[events[index].data_index++] = byte[2];
                events[index].data[events[index].data_index++] = (unsigned int)power(2, byte[3]);

                index++;
                i += 7;
                continue;
            }

            if (byte[1] == 0x51) {
                events[index].type = MIDI_EVENT_TEMPO;

                index++;
                i += 6;
                continue;
            }

            if (byte[1] == 0x2F) {
                events[index++].type = MIDI_EVENT_END;

                i += 3;
                break;
            }

            fprintf(stderr, "Unknown event 0xFF 0x%02X\n", byte[1]);
            rc = RETURN_EPERM;
            goto cleanup;
        }

        if ((byte[0] >> 4) == 0x8) {
            byte[1] = (unsigned int)data[i+1] & 0xFF;
            byte[2] = (unsigned int)data[i+1] & 0xFF;

            events[index].type = MIDI_EVENT_NOTE_OFF;
            events[index].data[events[index].data_index++] = byte[0] & 0xF;
            events[index].data[events[index].data_index++] = byte[1];
            events[index].data[events[index].data_index++] = byte[2];

            index++;

            i += 3;
            continue;
        }

        if ((byte[0] >> 4) == 0x9) {
            byte[1] = (unsigned int)data[i+1] & 0xFF;
            byte[2] = (unsigned int)data[i+1] & 0xFF;

            events[index].type = MIDI_EVENT_NOTE_ON;
            events[index].data[events[index].data_index++] = byte[0] & 0xF;
            events[index].data[events[index].data_index++] = byte[1];
            events[index].data[events[index].data_index++] = byte[2];

            index++;

            i += 3;
            continue;
        }

        if ((byte[0] >> 4) == 0xC) {
            events[index++].type = MIDI_EVENT_INSTRUMENT;

            i += 2;
            continue;
        }

        fprintf(stderr, "Unknown event 0x%02X\n", byte[0]);
        rc = RETURN_EPERM;
        goto cleanup;
    }

cleanup:
    track->events = events;
    track->event_count = event_count;

    return rc;
}

struct midi_data read_midi(char *filename) {
    unsigned int i = 0, l = 0;
    unsigned int track = 0;
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

        parse_events(&midi.tracks[track], midi.quarter);

        for (i = 0, l = midi.tracks[track].event_count; i < l; i++) {
            switch (midi.tracks[track].events[i].type) {
            case MIDI_EVENT_END:
                fprintf(stderr, "Track end\n");
                break;
            case MIDI_EVENT_TITLE:
                fprintf(stderr, "Track title\n");
                break;
            case MIDI_EVENT_TIME_SIGNATURE:
                fprintf(stderr, "Track time signature\n");
                break;
            case MIDI_EVENT_TEMPO:
                fprintf(stderr, "Track tempo\n");
                break;
            case MIDI_EVENT_NOTE_OFF:
                fprintf(stderr, "Beat %d - Channel %d: Note %d (%c) off at velocity %d\n", midi.tracks[track].events[i].beat, midi.tracks[track].events[i].data[0], midi.tracks[track].events[i].data[1], notes[midi.tracks[track].events[i].data[1] % 12], midi.tracks[track].events[i].data[2]);
                break;
            case MIDI_EVENT_NOTE_ON:
                fprintf(stderr, "Beat %d - Channel %d: Note %d (%c) on at velocity %d\n", midi.tracks[track].events[i].beat, midi.tracks[track].events[i].data[0], midi.tracks[track].events[i].data[1], notes[midi.tracks[track].events[i].data[1] % 12], midi.tracks[track].events[i].data[2]);
                break;
            case MIDI_EVENT_INSTRUMENT:
                fprintf(stderr, "Channel %d: Instrument %d\n", 0, 0);
                break;
            default:
                break;
            }
        }
    }

cleanup:
    return midi;
}
