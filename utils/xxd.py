#!/usr/bin/env python
# pylint: disable=C0103,C0301
"""Better xxd"""

import os
import re
import argparse

def xxd(infile, binary=False):
    """xxd"""

    output = []
    data = []
    slug = re.sub('[^a-zA-Z0-9]', '_', os.path.basename(infile))
    guard = '_%s' % (slug.replace('_' + os.path.splitext(infile)[1][1:], '_h').upper())
    text = []
    length = 0

    output.append('#ifndef %s' % (guard))
    output.append('#define %s\n' % (guard))

    with open(infile, 'r') as f:
        data = f.read()

    if binary:
        for i in range(0, len(data)):
            text.append('\\x%s' % (format(ord(data[i]), "x")))
    else:
        text.append(repr(data).strip('"\'').replace('"', '\\"'))

    output.append('unsigned char %s[] = "%s";' % (slug, ''.join(text)))

    length = len(data)

    output.append('unsigned int %s_len = %d;\n' % (slug, length))
    output.append('#endif /* %s */' % (guard))

    print '\n'.join(output)

def main():
    """Main function"""

    parser = argparse.ArgumentParser(description='Mapper scraper')
    parser.add_argument('--input', '-i', dest='infile', type=str, required=True, help='Input file')
    parser.add_argument('--binary', '-b', dest='binary', action="store_true", default=False, required=False, help='Binary')
    args = parser.parse_args()

    xxd(args.infile, args.binary)

if __name__ == '__main__':
    main()
