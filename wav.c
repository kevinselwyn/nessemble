#include <stdlib.h>
#include <string.h>
#include "nessemble.h"
#include "wav.h"

static unsigned int fgetu16(FILE *fp) {
    unsigned int a = (unsigned int)fgetc(fp);
    unsigned int b = (unsigned int)fgetc(fp);

    return a | (b << 8);
}

static long int fgetu32(FILE *fp) {
    unsigned int a = (unsigned int)fgetc(fp);
    unsigned int b = (unsigned int)fgetc(fp);
    unsigned int c = (unsigned int)fgetc(fp);
    unsigned int d = (unsigned int)fgetc(fp);

    return (long int)(a | (b << 8) | (c << 16) | (d << 24));
}

static int wav_format(wave_fmt *format, FILE *fp) {
    int rc = RETURN_OK;
    long int fmt_len = fgetu32(fp);

    if (fmt_len < 16) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    format->format = fgetu16(fp);
    format->channels = fgetu16(fp);
    format->sample_rate = fgetu32(fp);
    format->bytes_sec = fgetu32(fp);
    format->frame_size = fgetu16(fp);
    format->bits_sample = fgetu16(fp);

    (void)fseek(fp, fmt_len - 16, SEEK_CUR);

cleanup:
    return rc;
}

int wav_sample(wave_src *wav) {
    int i = 0, cur_sample = 0;

    if (wav->chunk_left == 0) {
        return 0;
    }

    for (i = 0; i < (int)wav->fmt.bits_sample && wav->chunk_left > 0; i += 8) {
        int c = fgetc(wav->fp);

        cur_sample >>= 8;
        cur_sample |= (c & 0xff) << 8;
        wav->chunk_left--;
    }

    if(wav->fmt.bits_sample <= 8) {
        cur_sample -= 32768;
    }

    cur_sample = (signed short)cur_sample;

    if (++wav->cur_chn >= (int)wav->fmt.channels) {
        wav->cur_chn = 0;
    }

    return cur_sample;
}

unsigned int open_wav(wave_src *wav, char *filename) {
    unsigned int rc = RETURN_OK, got_fmt = FALSE;
    size_t length = 0;
    char buffer[256];

    wav->fp = fopen(filename, "rb");

    length = strlen(cwd_path) + 1;

    if (!wav->fp) {
        yyerror("Could not open `%s`", filename+length);

        rc = RETURN_EPERM;
        goto cleanup;
    }

    if (fread(buffer, 1, 12, wav->fp) != 12) {
        yyerror("Could not read `%s`", filename+length);

        rc = RETURN_EPERM;
        goto cleanup;
    }

    if (memcmp("RIFF", buffer, 4) != FALSE || memcmp("WAVE", buffer + 8, 4) != FALSE) {
        yyerror("`%s` is not a WAV", filename+length);

        rc = RETURN_EPERM;
        goto cleanup;
    }

    while (fread(buffer, 4, 1, wav->fp) != 0) {
        if (memcmp("fmt ", buffer, 4) == FALSE) {
            int errc = wav_format(&(wav->fmt), wav->fp);
            if(errc < 0) {
                rc = RETURN_EPERM;
                goto cleanup;
            }

            got_fmt = TRUE;
        } else if (memcmp("data", buffer, 4) == FALSE) {
            if (got_fmt == FALSE) {
                rc = RETURN_EPERM;
                goto cleanup;
            }

            wav->chunk_left = fgetu32(wav->fp);

            if (wav->chunk_left == 0) {
                rc = RETURN_EPERM;
                goto cleanup;
            }

            wav->cur_chn = 0;

            rc = RETURN_OK;
            goto cleanup;
        } else {
            long int chunk_size = fgetu32(wav->fp);

            (void)fseek(wav->fp, chunk_size, SEEK_CUR);
        }
    }

    rc = RETURN_EPERM;

cleanup:
    return rc;
}

void close_wav(wave_src *wav) {
    if (wav->fp) {
        (void)fclose(wav->fp);
    }
}
