#!/usr/bin/env python
import sys

def main(argc, argv):
    output = 0

    for i in range(1, argc):
        output += int(argv[i])

    sys.stdout.write(chr(output % 256))
    exit(0)

if __name__ == '__main__':
    main(len(sys.argv), sys.argv)
