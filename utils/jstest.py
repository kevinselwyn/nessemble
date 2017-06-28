#!/usr/bin/env python
# pylint: disable=C0103,C0301
"""jstest"""

import os
import sys
import argparse

def gen_testfile(indir, excluded):
    """Generate testfile"""

    output = []
    tests = sorted(os.listdir(indir))

    # module name
    output.append('QUnit.module(\'%s\');\n' % (os.path.basename(indir)))

    # tests
    for test in tests:
        if not os.path.isdir(os.path.join(indir, test)):
            continue

        if test in excluded:
            continue

        input_text = ''

        with open(os.path.join(indir, test, '%s.asm' % (test)), 'r') as f:
            input_text = f.read()

        input_text = input_text.replace('\n', '\\n')
        input_text = input_text.replace('\'', '\\\'')

        output_bytes = []

        with open(os.path.join(indir, test, '%s.rom' % (test)), 'r') as f:
            output_bytes = bytearray(f.read())

        output_text = ''

        for byte in output_bytes:
            output_text += ', 0x%02x' % (byte)

        output_text = output_text[2:]

        output.append('\nQUnit.test(\'%s\', function (assert) {' % (test))
        output.append('    var done = assert.async(),')
        output.append('        input = \'%s\',' % (input_text))
        output.append('        output = [%s],' % (output_text))
        output.append('        nessemble = new Nessemble({')
        output.append('            stdin: input,')
        output.append('            onStdout: function (rc, stdout) {')
        output.append('                assert.deepEqual(stdout, output);')
        output.append('                done();')
        output.append('            },')
        output.append('            onStderr: function (rc, stderr) {')
        output.append('                var stderr_str = [],')
        output.append('                    output_str = [];\n')
        output.append('                stderr.forEach(function (byte) {')
        output.append('                    stderr_str.push(String.fromCharCode(byte));')
        output.append('                });\n')
        output.append('                output.forEach(function (byte) {')
        output.append('                    output_str.push(String.fromCharCode(byte));')
        output.append('                });\n')
        output.append('                assert.strictEqual(stderr_str.join(''), output_str.join(''));')
        output.append('                done();')
        output.append('            }')
        output.append('        });\n')
        output.append('    nessemble.callMain();')
        output.append('});')

    return '\n'.join(output)

def main():
    """Main function"""

    parser = argparse.ArgumentParser(description='Generates Javascript test file from test directory')
    parser.add_argument('--input', '-i', dest='indir', type=str, required=True, help='Input directory')
    parser.add_argument('--output', '-o', dest='outfile', type=str, required=False, default=None, help='Output file')
    parser.add_argument('--exclude', '-e', dest='exclude', type=str, required=False, default='', help='Exclude directory')
    args = parser.parse_args()

    if not args.outfile:
        output_filename = '.'.join([os.path.basename(args.indir), 'js'])
    else:
        output_filename = args.outfile

    if not os.path.isdir(args.indir):
        parser.print_help()
        sys.exit(1)

    input_dir = os.path.abspath(args.indir)
    output_data = gen_testfile(input_dir, args.exclude.split(','))

    with open(output_filename, 'w') as f:
        f.write(output_data)

if __name__ == '__main__':
    main()
