#ifndef _MIDI_H
#define _MIDI_H

enum {
    MIDI_FORMAT_0,
    MIDI_FORMAT_1,
    MIDI_FORMAT_2
};

enum {
    MIDI_META_EVENT_TRACK_COPYRIGHT = 0x02,
    MIDI_META_EVENT_TRACK_NAME = 0x03,
    MIDI_META_EVENT_DEVICE_NAME = 0x09,
    MIDI_META_EVENT_END = 0x2F,
    MIDI_META_EVENT_TEMPO = 0x51,
    MIDI_META_EVENT_SMPTE = 0x54,
    MIDI_META_EVENT_TIME_SIGNATURE = 0x58,
    MIDI_META_EVENT_KEY_SIGNATURE = 0x59,
    MIDI_EVENT_NOTE_OFF = 0x80,
    MIDI_EVENT_NOTE_ON = 0x90,
    MIDI_EVENT_INSTRUMENT = 0xC0
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
