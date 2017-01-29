#ifndef _WAV_H
#define _WAV_H

#include <stdio.h>

typedef struct wave_fmt {
    unsigned int format;
    unsigned int channels;
    long int sample_rate;
    long int bytes_sec;
    unsigned int frame_size;
    unsigned int bits_sample;
} wave_fmt;

typedef struct wave_src {
    wave_fmt fmt;
    FILE *fp;
    long int chunk_left;
    int cur_chn;
} wave_src;

int wav_sample(wave_src *wav);
unsigned int open_wav(wave_src *wav, char *filename);
void close_wav(wave_src *wav);

#endif /* _WAV_H */
