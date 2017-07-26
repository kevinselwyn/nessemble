interface AssemblersHexdumpLine {
    index: string;
    bytes: string[][];
    text: string[];
}

class StringHelper {
    static dec2hex(dec: number, len: number): string {
        var hex: string = dec.toString(16).toLowerCase();

        while (hex.length < len) {
            hex = '0' + hex;
        }

        return hex;
    }

    static hexdump(bytes: number[]): string {
        var dump: string[] = [],
            index: number = 0,
            lines: AssemblersHexdumpLine[] = [],
            line: AssemblersHexdumpLine,
            chr: string = '',
            i: number = 0,
            j: number = 0,
            k: number = 0,
            l: number = 0;

        for (i = 0, j = bytes.length; i < j; i += 16) {
            line = {
                index: this.dec2hex(i, 8),
                bytes: [[], []],
                text: []
            };

            for (k = i, l = i + 8; k < l; k += 1) {
                if (k >= j) {
                    break;
                }

                index += 1;
                line.bytes[0].push(this.dec2hex(bytes[k], 2));

                if (bytes[k] < 0x20 || bytes[k] > 0x7E) {
                    chr = '.';
                } else {
                    chr = String.fromCharCode(bytes[k]);
                }

                line.text.push(chr);
            }

            for (k = i + 8, l = i + 16; k < l; k += 1) {
                if (k >= j) {
                    break;
                }

                index += 1;
                line.bytes[1].push(this.dec2hex(bytes[k], 2));

                if (bytes[k] < 0x20 || bytes[k] > 0x7E) {
                    chr = '.';
                } else {
                    chr = String.fromCharCode(bytes[k]);
                }

                line.text.push(chr);
            }

            lines.push(line);
        }

        lines.push({
            index: this.dec2hex(index, 8),
            bytes: [[], []],
            text: []
        });

        lines.forEach(function (line: AssemblersHexdumpLine): void {
            var str: string = '';

            str = [line.index, line.bytes[0].join(' '), line.bytes[1].join(' ')].join('  ');

            while (str.length < 60) {
                str += ' ';
            }

            if (line.text.length) {
                str += ['|', line.text.join(''), '|'].join('');
            }

            dump.push(str);
        });

        return dump.join('\n');
    }
}

export default StringHelper;
