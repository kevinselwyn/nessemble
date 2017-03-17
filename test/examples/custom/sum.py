#!/usr/bin/python
# coding=utf-8
"""sum custom pseudo-instruction"""

import sys

def main():
    output = 0

    for num in sys.argv[1:]:
        output += int(num)

    for char in str(output):
        sys.stdout.write(chr(int(char)))

    exit(0)

if __name__ == '__main__':
    main()
