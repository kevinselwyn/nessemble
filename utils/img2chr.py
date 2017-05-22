#!/usr/bin/env python
# pylint: disable=C0103,C0301
"""img2chr"""

import math
import sys
import argparse
from PIL import Image

def load_img(filename):
    """Loads image"""

    img = Image.open(filename)

    return img

def save_chr(filename, data):
    """Saves chr"""

    with open(filename, 'wb') as f:
        f.write(data)

def get_color(pixel):
    """Gets normalized color of pixel"""

    colors = [0, 0x55, 0xAA, 0xFF]
    rgb = math.floor((pixel[0] + pixel[1] + pixel[2]) / 3)
    color = colors.index(min(colors, key=lambda x: abs(x - rgb)))

    return color

def str2bin(string):
    """Converts binary string to binary int"""

    output = bytearray()

    for x in range(0, len(string), 8):
        output.append(int(string[x:x+8], 2))

    return output

def img2bin(img):
    """Converts image to binary"""

    width, height = img.size
    pixel = img.load()
    color = 0
    binary = ''

    for h in range(0, height, 8):
        for w in range(0, width, 8):
            lo = ''
            hi = ''

            for y in range(h, h + 8):
                for x in range(w, w + 8):
                    color = get_color(pixel[x, y])
                    lo += str(color & 0x01)
                    hi += str((color >> 1) & 0x01)

            binary += lo + hi

    return binary

def main():
    """Main function"""

    parser = argparse.ArgumentParser(description='Converts images to the NES CHR format for programming NES ROMs in 6502 assembly')
    parser.add_argument('--input', '-i', dest='infile', type=str, required=True, help='Input file')
    parser.add_argument('--output', '-o', dest='outfile', type=str, required=False, default=None, help='Output file')
    args = parser.parse_args()

    input_filename = args.infile

    if not args.outfile:
        output_filename = '.'.join([input_filename.rsplit('.', 1)[0], 'chr'])
    else:
        output_filename = args.outfile

    if input_filename.endswith('.png') != True:
        print 'Invalid image extension'
        sys.exit(1)

    img = load_img(input_filename)
    binary = img2bin(img)
    output = str2bin(binary)
    save_chr(output_filename, output)

if __name__ == '__main__':
    main()
