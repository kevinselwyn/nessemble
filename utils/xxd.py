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

    output.append('#ifndef %s' % (guard))
    output.append('#define %s\n' % (guard))

    with open(infile, 'r') as f:
        data = f.read()

    output.append('unsigned char %s[] = "%s";' % (slug, repr(data).strip('"\'').replace('"', '\\"')))

    length = len(data)

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
