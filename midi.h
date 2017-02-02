#ifndef _MIDI_H
#define _MIDI_H

enum {
    MIDI_FORMAT_0,
    MIDI_FORMAT_1,
    MIDI_FORMAT_2
};

enum {
    MIDI_EVENT_END,
    MIDI_EVENT_TITLE,
    MIDI_EVENT_TIME_SIGNATURE,
    MIDI_EVENT_TEMPO,
    MIDI_EVENT_NOTE_OFF,
    MIDI_EVENT_NOTE_ON,
    MIDI_EVENT_INSTRUMENT
};

struct midi_data read_midi(char *filename);

struct midi_event {
    unsigned int type, beat, data[3], data_index;
};

struct midi_track {
    char *header[4], *data;
    unsigned int length, event_count;
    struct midi_event *events;
};

struct midi_data {
    char *header[4];
    unsigned int header_length, format, track_count, quarter;
    struct midi_track *tracks;
};

#endif /* _MIDI_H */
