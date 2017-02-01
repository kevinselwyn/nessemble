#ifndef _MIDI_H
#define _MIDI_H

enum {
    MIDI_FORMAT_0,
    MIDI_FORMAT_1,
    MIDI_FORMAT_2
};

struct midi_data read_midi(char *filename);

struct midi_data {
    char *header[4];
    unsigned int header_length, format, tracks, quarter;
};

#endif /* _MIDI_H */
