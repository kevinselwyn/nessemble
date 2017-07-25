declare type ModuleType = (obj: ModuleObj) => any;
declare type ModuleObjExit = (rc: number) => void;
declare type ModuleObjStdin = () => void;
declare type ModuleObjStd = (code: number) => void;

interface ModuleInterface {
    callMain?: any;
}

interface ModuleObj {
    noInitialRun: boolean;
    onExit: ModuleObjExit;
    stdin: ModuleObjStdin;
    stdout: ModuleObjStd;
    stderr: ModuleObjStd;
}

declare type NessembleStd = (rc: number, data: number[]) => void;

interface NessembleOpts {
    stdin: any;
    onStdout: NessembleStd;
    onStderr: NessembleStd;
}

class Nessemble {
    Module: ModuleType;

    constructor(opts: NessembleOpts) {
        var Module: ModuleType = null,
            stdout: number[] = [],
            stderr: number[] = [],
            i: number = 0;

        if (!(<any>window).Module) {
            return;
        }

        Module = (<any>window).Module;

        this.Module = <ModuleType>Module({
            noInitialRun: true,
            onExit: function (rc: number): void {
                if (stdout.length && opts.onStdout) {
                    opts.onStdout(rc, stdout);
                }

                if (stderr.length && opts.onStderr) {
                    opts.onStderr(rc, stderr);
                }
            },
            stdin: function (): number|null {
                if (i < (opts.stdin || '').length) {
                    var code = opts.stdin.charCodeAt(i);

                    i += 1;

                    return code;
                } else {
                    return null;
                }
            },
            stdout: function (code: number): void {
                if (code < 0) {
                    code += 256;
                }

                stdout.push(code);
            },
            stderr: function (code: number): void {
                stderr.push(code);
            }
        });
    }

    callMain(args: string[]): void {
        (<ModuleInterface>this.Module).callMain(args);
    }
}

interface NessembleExamplesOpts {
    args?: string[];
    bare?: boolean;
    disassemble?: boolean;
    download?: boolean;
    flags?: string[];
}

interface NessembleExamplesHexdumpLine {
    index: string;
    bytes: string[][];
    text: string[];
}

class NessembleExamples {
    constructor() {
        var examples: NodeListOf<Node> = document.querySelectorAll('.nessemble-example');

        if (!(<any>window).Module) {
            return;
        }

        if (!examples || !examples.length) {
            return;
        }

        [].forEach.call(examples, (example: Node) => {
            this.setup(<HTMLElement>example);
        });
    }

    addClass(el: HTMLElement, className: string): void {
        var el_classname: string = el.className,
            regex: RegExp = new RegExp(['\\b', className, '\\b'].join(''));

        if (el_classname.search(regex) === -1) {
            el_classname += ' ' + className;
            el_classname = el_classname.replace(/\s{2,}/, ' ');
            el_classname = el_classname.replace(/^\s+/, '');
            el_classname = el_classname.replace(/\s+$/, '');

            el.className = el_classname;
        }
    }

    removeClass(el: HTMLElement, className: string): void {
        var el_classname: string = el.className,
            regex: RegExp = new RegExp(['\\b', className, '\\b'].join(''));

        if (el_classname.search(regex) !== -1) {
            el_classname = el_classname.replace(regex, '');
            el_classname = el_classname.replace(/^\s+/, '');
            el_classname = el_classname.replace(/\s+$/, '');

            el.className = el_classname;
        }
    }

    downloadFile(filename: string, bytes: number[], mimetype: string): void {
        var arr: Uint8Array = new Uint8Array(bytes),
            el: HTMLAnchorElement = document.createElement('a');

        el.href = window.URL.createObjectURL(new Blob([arr], {
            type: mimetype
        }));
        el.download = filename;

        document.body.appendChild(el);
        el.click();
        document.body.removeChild(el);
    }

    dec2hex(dec: number, len: number): string {
        var hex: string = dec.toString(16).toLowerCase();

        while (hex.length < len) {
            hex = '0' + hex;
        }

        return hex;
    }

    hexdump(bytes: number[]): string {
        var $this = this,
            dump: string[] = [],
            index: number = 0,
            lines: NessembleExamplesHexdumpLine[] = [],
            line: NessembleExamplesHexdumpLine,
            chr: string = '',
            i: number = 0,
            j: number = 0,
            k: number = 0,
            l: number = 0;

        for (i = 0, j = bytes.length; i < j; i += 16) {
            line = {
                index: $this.dec2hex(i, 8),
                bytes: [[], []],
                text: []
            };

            for (k = i, l = i + 8; k < l; k += 1) {
                if (k >= j) {
                    break;
                }

                index += 1;
                line.bytes[0].push($this.dec2hex(bytes[k], 2));

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
                line.bytes[1].push($this.dec2hex(bytes[k], 2));

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
            index: $this.dec2hex(index, 8),
            bytes: [[], []],
            text: []
        });

        lines.forEach(function (line: NessembleExamplesHexdumpLine): void {
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

    setup(example: HTMLElement) {
        var $this = this,
            opts: NessembleExamplesOpts = {},
            args: string[] = [],
            rc: number = 0,
            stdin: string = example.innerHTML,
            stdout: number[] = [],
            stderr: number[] = [],
            hr: HTMLHRElement = document.createElement('hr'),
            paragraph: HTMLParagraphElement = document.createElement('p'),
            exec: HTMLTextAreaElement = document.createElement('textarea'),
            input: HTMLTextAreaElement = document.createElement('textarea'),
            buttons: HTMLDivElement = document.createElement('div'),
            assemble: HTMLButtonElement = document.createElement('button'),
            reset: HTMLButtonElement = document.createElement('button'),
            clear: HTMLButtonElement = document.createElement('button'),
            download: HTMLButtonElement = document.createElement('button'),
            open: HTMLButtonElement = document.createElement('button'),
            help: HTMLButtonElement = document.createElement('button'),
            output_wrapper: HTMLPreElement = document.createElement('pre'),
            output: HTMLElement = document.createElement('code'),
            element: HTMLDivElement = document.createElement('div'),
            modal_title: HTMLElement = <HTMLElement>document.querySelector('.modal-header'),
            modal_body: HTMLElement = <HTMLElement>document.querySelector('.modal-body');

        // quit if already initialized
        if (example.getAttribute('data-initialized') === 'true') {
            return;
        }

        // get data-opts
        try {
            opts = JSON.parse(example.getAttribute('data-opts')) || {};
        } catch (e) {
            opts = {};
        }

        // get from localStorage
        if (opts.bare) {
            if (window.localStorage) {
                stdin = window.localStorage.getItem('asm') || stdin;
                window.localStorage.removeItem('asm');
            }
        }

        // get args
        args = opts.args || [];

        // decode html entities
        element.innerHTML = stdin;
        stdin = element.textContent;

        // assemble click listener
        assemble.addEventListener('click', function (): void {
            var nessemble: Nessemble = new Nessemble({
                stdin: (function (): string {
                    var str: string = input.value || '',
                        new_str: string[] = [],
                        bytes: string[] = [];

                    if (opts.disassemble) {
                        bytes = str.split(',');

                        bytes.forEach(function (byte: string, i: number): void {
                            byte = byte.replace('0x', '');
                            byte = byte.replace(/^\s+/, '');
                            byte = byte.replace(/\s+$/, '');

                            new_str.push(String.fromCharCode(parseInt(byte, 16)));
                        });

                        str = new_str.join('');
                    }

                    return str;
                }()),
                onStdout: function (_rc: number, _data: number[]): void {
                    rc = _rc;
                    stdout = _data;

                    output.innerHTML = '';
                    $this.removeClass(output, 'error');

                    if (!stdout.length) {
                        $this.removeClass(output, 'show');
                        download.disabled = true;
                        return;
                    }

                    if (opts.flags || opts.disassemble) {
                        stdout.forEach(function (byte: number): void {
                            output.innerHTML += String.fromCharCode(byte);
                        });
                    } else {
                        output.innerHTML = $this.hexdump(stdout);
                    }

                    $this.addClass(output, 'show');

                    download.removeAttribute('disabled');
                },
                onStderr: function (_rc: number, _data: number[]): void {
                    rc = _rc;
                    stderr = _data;

                    output.innerHTML = '';
                    $this.removeClass(output, 'error');
                    download.disabled = true;

                    if (!stderr.length) {
                        $this.removeClass(output, 'show');
                        return;
                    }

                    stderr.forEach(function (byte: number): void {
                        output.innerHTML += String.fromCharCode(byte);
                    });

                    $this.addClass(output, 'error');
                    $this.addClass(output, 'show');
                }
            });

            if (opts.flags) {
                args = input.value.split(' ');
            }

            nessemble.callMain(args || []);

            ga('send', 'event', 'Assembler', 'Assemble', 'Documentation', {
                dimension3: input.value
            });
        });

        // reset click listener
        reset.addEventListener('click', function (): void {
            // reset input
            input.value = stdin;

            // clear output
            output.innerHTML = '';
            $this.removeClass(output, 'show');
            $this.removeClass(output, 'error');
            download.disabled = true;

            ga('send', 'event', 'Assembler', 'Reset', 'Documentation', {
                dimension3: input.value
            });
        });

        // clear click listener
        clear.addEventListener('click', function (): void {
            ga('send', 'event', 'Assembler', 'Clear', 'Documentation', {
                dimension3: input.value
            });

            // clear input
            input.value = '';

            // clear output
            output.innerHTML = '';
            $this.removeClass(output, 'show');
            $this.removeClass(output, 'error');
            download.disabled = true;
        });

        // download click listener
        download.addEventListener('click', function (): void {
            $this.downloadFile('assemble.rom', stdout, 'application/octet-stream');

            ga('send', 'event', 'Assembler', 'Download', 'Documentation');
        });

        // open click listener
        open.addEventListener('click', function (): void {
            if (window.localStorage) {
                window.localStorage.setItem('asm', input.value);
            }

            ga('send', 'event', 'Assembler', 'Open', 'Documentation', {
                dimension3: input.value
            });

            window.location.href = open.getAttribute('data-href');
        });

        // clear out element
        example.innerHTML = '';

        // add paragraph
        paragraph.innerHTML = 'Try it:';

        // add stdin to textarea
        input.value = stdin;
        $this.addClass(input, 'input');

        // add exec
        $this.addClass(exec, 'exec');
        exec.innerHTML = 'nessemble';
        exec.disabled = true;

        // button group
        $this.addClass(buttons, 'btn-group');

        // setup assemble button
        if (opts.flags) {
            assemble.innerHTML = 'Run';
        } else if (opts.disassemble) {
            assemble.innerHTML = 'Disassemble';
        } else {
            assemble.innerHTML = 'Assemble';
        }

        $this.addClass(assemble, 'btn');
        $this.addClass(assemble, 'btn-danger');
        assemble.setAttribute('title', 'Assemble');

        // reset clear button
        reset.innerHTML = 'Reset';
        $this.addClass(reset, 'btn');
        $this.addClass(reset, 'btn-neutral');
        reset.setAttribute('title', 'Reset');

        // setup clear button
        clear.innerHTML = 'Clear';
        $this.addClass(clear, 'btn');
        $this.addClass(clear, 'btn-neutral');
        clear.setAttribute('title', 'Clear');

        // setup download button
        download.innerHTML = 'Download';
        $this.addClass(download, 'btn');
        $this.addClass(download, 'btn-neutral');
        download.disabled = true;
        download.setAttribute('title', 'Download');

        // setup open button
        open.innerHTML = '<i class="fa fa-external-link"></i>';
        $this.addClass(open, 'btn');
        $this.addClass(open, 'btn-neutral');
        open.setAttribute('data-href', '../playground');
        open.setAttribute('title', 'Open in Playground');

        // setup help button
        help.innerHTML = '<i class="fa fa-question-circle"></i>';
        $this.addClass(help, 'btn');
        $this.addClass(help, 'btn-neutral');
        help.setAttribute('title', 'Help');
        help.setAttribute('data-toggle', 'modal');
        help.setAttribute('data-target', '#help-modal');

        // setup help modal
        modal_title.innerHTML = 'Nessemble Help';
        modal_body.innerHTML = '<p><code>nessemble</code> has been compiled to Javascript for use in the browser (with limited functionality).</p><p>Feel free to explore <code>nessemble</code> further in the <a href="../playground">Playground</a>.</p>';

        // append buttons
        buttons.appendChild(assemble);

        if (!opts.flags) {
            buttons.appendChild(reset);
            buttons.appendChild(clear);

            if (opts.download) {
                buttons.appendChild(download);
            }

            if (!opts.bare && !opts.disassemble) {
                buttons.appendChild(open);
            }

            buttons.appendChild(help);
        }

        // setup output
        $this.addClass(output, 'text');
        output_wrapper.appendChild(output);

        // append all
        if (!opts.bare) {
            example.appendChild(hr);
            example.appendChild(paragraph);
        }

        if (opts.flags) {
            example.appendChild(exec);
        }

        example.appendChild(input);
        example.appendChild(buttons);
        example.appendChild(output_wrapper);

        // show module
        $this.addClass(example, 'show');

        // denote bare display
        if (opts.bare) {
            $this.addClass(example, 'bare');
        }

        // denote flags display
        if (opts.flags) {
            $this.addClass(example, 'flags');
        }

        // mark as initialized
        example.setAttribute('data-initalized', 'true');
    }
}

var nessembleExamples: NessembleExamples = new NessembleExamples();
