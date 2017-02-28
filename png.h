#include <png.h>

struct png_data {
	int *bits, width, height;
    char *header[8];
	unsigned char color_type, bit_depth;
	png_structp png_ptr;
	png_infop info_ptr;
	unsigned char **row_pointers;
};

void free_png(struct png_data png);
void free_png_read(struct png_data png);
void free_png_write(struct png_data png);
int png_color_mode(int color_type);
struct png_data read_png(char *filename);
unsigned int write_png(unsigned int *pixels, unsigned int width, unsigned int height, char *filename);
int get_color(unsigned char *rgb, int color_mode);
int match_color(unsigned char *rgb, int color_mode);
