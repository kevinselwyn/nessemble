#!/usr/bin/env python
# pylint: disable=C0103
"""GUID generator"""

import uuid
import argparse

def guid(infile='', string=''):
    """GUID"""

    data = ''

    if infile:
        with open(infile, 'r') as f:
            data = f.read()
    else:
        data = string

    print uuid.uuid3(uuid.NAMESPACE_OID, data)

def main():
    """Main function"""

    parser = argparse.ArgumentParser(description='GUID generator')
    parser.add_argument('--input', dest='infile', type=str, required=False, help='Input file')
    parser.add_argument('--string', dest='string', type=str, required=False, help='Input string')
    args = parser.parse_args()

    if not args.infile and not args.string:
        parser.print_help()
        exit(1)

    guid(args.infile, args.string)

if __name__ == '__main__':
    main()
