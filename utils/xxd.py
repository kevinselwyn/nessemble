#!/usr/bin/env python
# pylint: disable=C0103
"""Better xxd"""

import re
import argparse

def xxd(infile):
    """xxd"""

    output = []
    data = []
    slug = re.sub('[^a-zA-Z0-9]', '_', infile)
    guard = '_%s' % (slug.replace('_txt', '_h').upper())
    length = 0
    line = ''

    output.append('#ifndef %s' % (guard))
    output.append('#define %s\n' % (guard))
    output.append('unsigned char %s[] = {' % (slug))

    with open(infile, 'r') as f:
        data = f.read()

    length = len(data)

    for i in range(0, length, 12):
        line = '  '

        for j in range(0, 12):
            if i + j == length:
                break

            line += '0x%02x' % (ord(data[i+j]))

            if (i + j) + 1 != length:
                line += ', '

        output.append(line)

    output.append('};')
    output.append('unsigned int %s_len = %d;\n' % (slug, length))
    output.append('#endif /* %s */' % (guard))

    print '\n'.join(output)

def main():
    """Main function"""

    parser = argparse.ArgumentParser(description='Mapper scraper')
    parser.add_argument('--input', '-i', dest='infile', type=str, required=True, help='Input file')
    args = parser.parse_args()

    xxd(args.infile)

if __name__ == '__main__':
    main()
