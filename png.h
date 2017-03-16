#ifndef _PNG_H
#define _PNG_H

struct png_data {
    unsigned int width, height;
	unsigned char *data;
};

void free_png(struct png_data png);
struct png_data read_png(char *filename);
unsigned int write_png(unsigned int *pixels, unsigned int width, unsigned int height, char *filename);
int get_color(unsigned char *rgb, int color_mode);
int match_color(unsigned char *rgb);

#endif /* _PNG_H */
