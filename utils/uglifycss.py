#!/usr/bin/env python
# pylint: disable=C0301
"""CSS uglifier"""

import argparse
from cssmin import cssmin

def uglify(infiles=None, outfile=None):
    """Uglify CSS"""

    if not infiles:
        return

    data = ''

    for infile in infiles:
        with open(infile, 'r') as css_file:
            data += css_file.read()

    minified = cssmin(data)

    if not outfile:
        print minified
        return

    with open(outfile, 'w') as cssmin_file:
        cssmin_file.write(minified)

def main():
    """Main function"""

    parser = argparse.ArgumentParser(description='CSS uglifier')
    parser.add_argument('--output', '-o', dest='outfile', type=str, default=None, required=False, help='Output file')
    parser.add_argument('infiles', type=str, nargs='*', help='Input files')
    args = parser.parse_args()

    uglify(args.infiles, args.outfile)

if __name__ == '__main__':
    main()
