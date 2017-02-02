#ifndef _MIDI_H
#define _MIDI_H

enum {
    MIDI_FORMAT_0,
    MIDI_FORMAT_1,
    MIDI_FORMAT_2
};

struct midi_data read_midi(char *filename);

struct midi_event {
    unsigned int type;
};

struct midi_track {
    char *header[4], *data;
    unsigned int length;
    struct midi_event *events;
};

struct midi_data {
    char *header[4];
    unsigned int header_length, format, track_count, quarter;
    struct midi_track *tracks;
};

#endif /* _MIDI_H */
