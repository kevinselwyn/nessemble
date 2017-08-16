#!/usr/bin/env python
# pylint: disable=C0301
"""Javascript uglifier"""

import argparse
from jsmin import jsmin

def uglify(infiles=None, outfile=None):
    """Uglify Javascript"""

    if not infiles:
        return

    data = ''

    for infile in infiles:
        with open(infile, 'r') as js_file:
            data += js_file.read()

    minified = jsmin(data)

    if not outfile:
        print minified
        return

    with open(outfile, 'w') as jsmin_file:
        jsmin_file.write(minified)

def main():
    """Main function"""

    parser = argparse.ArgumentParser(description='Javascript uglifier')
    parser.add_argument('--output', '-o', dest='outfile', type=str, default=None, required=False, help='Output file')
    parser.add_argument('infiles', type=str, nargs='*', help='Input files')
    args = parser.parse_args()

    uglify(args.infiles, args.outfile)

if __name__ == '__main__':
    main()
