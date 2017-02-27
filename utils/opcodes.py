#!/usr/bin/env python
"""Opcode parser"""

import argparse

def opcodes(infile):
    lines = []
    output = []

    with open(infile, "r") as f:
        for line in f:
            lines.append(line.strip('\n').split(','))

    output.append('#include "nessemble.h"\n\n')
    output.append('struct opcode opcodes[OPCODE_COUNT] = {\n')

    for i in range(0, len(lines)):
        [mnemonic, mode, opcode, length, timing, meta] = lines[i]
        meta_str = ''
        meta_arr = []
        func = mnemonic.lower().strip('"')

        if meta == '0':
            meta_str = 'META_NONE'
        else:
            if (int(meta) & 1) > 0:
                meta_arr.append('META_BOUNDARY')

            if (int(meta) & 2) > 0:
                meta_arr.append('META_UNDOCUMENTED')

        if len(meta_arr):
            meta_str = ' | '.join(meta_arr)

        output.append('    { %-06s %-17s %s, %s, %s, %-34s &do_%s }' % (mnemonic + ',', mode + ',', opcode, length, timing, meta_str + ',', func))

        if i + 1 < len(lines):
            output.append(',')

        output.append('\n')

    output.append('};\n')

    print ''.join(output)

def main():
    """Main function"""

    parser = argparse.ArgumentParser(description='Opcode parser')
    parser.add_argument('--input', '-i', dest='infile', type=str, required=True, help='Input file')
    args = parser.parse_args()

    opcodes(args.infile)

if __name__ == '__main__':
    main()
