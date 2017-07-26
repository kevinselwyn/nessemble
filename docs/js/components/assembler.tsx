import * as React from 'react';
import * as autoBind from 'react-autobind';
import * as classNames from 'classnames';

import Assembler from '../models/assembler';

interface AssemblerComponentProps {
    args?: string[];
    bare?: boolean;
    disassemble?: boolean;
    download?: boolean;
    flags?: string[];
    pseudo?: string[];
    stdin?: string;
}

interface AssemblersHexdumpLine {
    index: string;
    bytes: string[][];
    text: string[];
}

class AssemblerComponent extends React.Component<AssemblerComponentProps, any> {
    stdout: number[];
    stderr: number[];

    constructor(props: AssemblerComponentProps) {
        super(props);
        autoBind(this);

        var stdin = props.stdin;

        if (this.props.bare) {
            if (window.localStorage) {
                stdin = window.localStorage.getItem('asm') || stdin;
                window.localStorage.removeItem('asm');
            }
        }

        this.state = {
            download: false,
            input: stdin,
            output: '',
            output_show: false,
            output_error: false,
            pseudos: false,
            pseudo: props.pseudo
        };

        this.stdout = [];
        this.stderr = [];
    }

    componentDidMount() {
        var modal_title = document.querySelector('.modal-header'),
            modal_body = document.querySelector('.modal-body');

        modal_title.innerHTML = 'Nessemble Help';
        modal_body.innerHTML = '<p><code>nessemble</code> has been compiled to Javascript for use in the browser (with limited functionality).</p><p>Feel free to explore <code>nessemble</code> further in the <a href="../playground">Playground</a>.</p>';
    }

    _onChangeInput(e: any) {
        this.setState({
            input: e.target.value
        });
    }

    _onChangePseudo(pseudo: string) {
        var pseudo_obj = this.state.pseudo;

        pseudo_obj[pseudo] = !pseudo_obj[pseudo];

        this.setState({
            pseudo: pseudo_obj
        });
    }

    _onAssemble() {
        var $this = this,
            args = this.props.args,
            assembler: Assembler = new Assembler({
                stdin: (function (): string {
                    var str: string = $this.state.input,
                        new_str: string[] = [],
                        bytes: string[] = [];

                    if ($this.props.disassemble) {
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
                    var download = $this.state.download,
                        output = $this.state.output,
                        output_show = $this.state.output_show,
                        output_error = $this.state.output_error;

                    $this.stdout = _data;

                    output = '';
                    output_error = false;

                    if (!$this.stdout.length) {
                        output_show = false;
                        download = false;

                        $this.setState({
                            download: download,
                            output: output,
                            output_show: output_show,
                            output_error: output_error
                        });

                        return;
                    }

                    if ($this.props.flags || $this.props.disassemble) {
                        $this.stdout.forEach(function (byte: number): void {
                            output += String.fromCharCode(byte);
                        });
                    } else {
                        output = $this._hexdump($this.stdout);
                    }

                    output_show = true;
                    download = true;

                    $this.setState({
                        download: download,
                        output: output,
                        output_show: output_show,
                        output_error: output_error
                    });
                },
                onStderr: function (_rc: number, _data: number[]): void {
                    var download = $this.state.download,
                        output = $this.state.output,
                        output_show = $this.state.output_show,
                        output_error = $this.state.output_error;

                    $this.stderr = _data;

                    output = '';
                    output_error = false;
                    download = false;

                    if (!$this.stderr.length) {
                        output_show = false;

                        $this.setState({
                            download: download,
                            output: output,
                            output_show: output_show,
                            output_error: output_error
                        });

                        return;
                    }

                    $this.stderr.forEach(function (byte: number): void {
                        output += String.fromCharCode(byte);
                    });

                    output_error = true;
                    output_show = true;

                    $this.setState({
                        download: download,
                        output: output,
                        output_show: output_show,
                        output_error: output_error
                    });
                },
                custom: (function (): object {
                    var obj: any = {};

                    if (!$this.state.pseudo) {
                        return obj;
                    }

                    Object.keys($this.state.pseudo).forEach((pseudo) => {
                        if ($this.state.pseudo[pseudo]) {
                            obj[`.${pseudo}`] = `/static/scripts/${pseudo}.lua`;
                        }
                    });

                    return obj;
                }())
            });

        if (this.props.flags) {
            args = this.state.input.split(' ');
        }

        assembler.callMain(args || []);

        ga('send', 'event', 'Assembler', 'Assemble', 'Documentation', {
            dimension3: this.state.input
        });
    }

    _onReset() {
        this.setState({
            input: this.props.stdin,
            output_show: false,
            output_error: false,
            download: false
        });

        ga('send', 'event', 'Assembler', 'Reset', 'Documentation', {
            dimension3: this.props.stdin
        });
    }

    _onClear() {
        ga('send', 'event', 'Assembler', 'Clear', 'Documentation', {
            dimension3: this.state.input
        });

        this.setState({
            input: '',
            output_show: false,
            output_error: false,
            download: false
        });
    }

    _onDownload() {
        var filename: string = 'assemble.rom',
            mimetype = 'application/octet-stream',
            arr: Uint8Array = new Uint8Array(this.stdout),
            el: HTMLAnchorElement = document.createElement('a');

        el.href = window.URL.createObjectURL(new Blob([arr], {
            type: mimetype
        }));
        el.download = filename;

        document.body.appendChild(el);
        el.click();
        document.body.removeChild(el);

        ga('send', 'event', 'Assembler', 'Download', 'Documentation');
    }

    _onCustom() {
        this.setState({
            pseudos: !this.state.pseudos
        });

        ga('send', 'event', 'Assembler', 'Toggle Pseudo', 'Documentation', {
            dimension3: this.state.input
        });
    }

    _onOpen() {
        if (window.localStorage) {
            window.localStorage.setItem('asm', this.state.input);
        }

        ga('send', 'event', 'Assembler', 'Open', 'Documentation', {
            dimension3: this.state.input
        });

        window.location.href = '../playground/';
    }

    _dec2hex(dec: number, len: number): string {
        var hex: string = dec.toString(16).toLowerCase();

        while (hex.length < len) {
            hex = '0' + hex;
        }

        return hex;
    }

    _hexdump(bytes: number[]): string {
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
                index: this._dec2hex(i, 8),
                bytes: [[], []],
                text: []
            };

            for (k = i, l = i + 8; k < l; k += 1) {
                if (k >= j) {
                    break;
                }

                index += 1;
                line.bytes[0].push(this._dec2hex(bytes[k], 2));

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
                line.bytes[1].push(this._dec2hex(bytes[k], 2));

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
            index: this._dec2hex(index, 8),
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

    render() {
        var assemble_text: string = 'Assemble';

        if (this.props.flags) {
            assemble_text = 'Run';
        } else if (this.props.disassemble) {
            assemble_text = 'Disassemble';
        } else {
            assemble_text = 'Assemble';
        }

        return (
            <div>
                {
                    !this.props.bare ? (
                        <hr />
                    ) : null
                }
                {
                    !this.props.bare ? (
                        <p>Try it:</p>
                    ) : null
                }
                {
                    this.props.flags ? (
                        <textarea
                            className="exec"
                            disabled={true}
                            readOnly={true}
                            value="nessemble" />
                    ) : null
                }
                <textarea
                    className="input"
                    value={this.state.input}
                    onChange={this._onChangeInput} />
                {
                    this.props.pseudo ? (
                        <form
                            className={classNames({
                                show: this.state.pseudos
                            })}>
                            {
                                Object.keys(this.state.pseudo).map((pseudo, i) => {
                                    return (
                                        <div
                                            key={i}
                                            className="pseudo-opt">
                                            <input
                                                id={`pseudo-opt-${pseudo}`}
                                                type="checkbox"
                                                checked={this.state.pseudo[pseudo]}
                                                value={pseudo}
                                                onChange={() => {
                                                    this._onChangePseudo(pseudo);
                                                }} />
                                            <label
                                                htmlFor={`pseudo-opt-${pseudo}`}>.{pseudo}</label>
                                        </div>
                                    );
                                })
                            }
                        </form>
                    ) : null
                }
                <div
                    className="btn-group">
                    <button
                        className={classNames({
                            btn: true,
                            'btn-danger': true
                        })}
                        title={assemble_text}
                        onClick={this._onAssemble}>{assemble_text}</button>
                    {
                        this.props.flags ? (
                            <button
                                className={classNames({
                                    btn: true,
                                    'btn-neutral': true
                                })}
                                title="Reset"
                                onClick={this._onReset}>Reset</button>
                        ) : null
                    }
                    {
                        !this.props.flags ? (
                            <button
                                className={classNames({
                                    btn: true,
                                    'btn-neutral': true
                                })}
                                title="Clear"
                                onClick={this._onClear}>Clear</button>
                        ) : null
                    }
                    {
                        !this.props.flags ? (
                            this.props.download ? (
                                <button
                                    className={classNames({
                                        btn: true,
                                        'btn-neutral': true
                                    })}
                                    disabled={!this.state.download}
                                    title="Download"
                                    onClick={this._onDownload}>Download</button>
                            ) : null
                        ) : null
                    }
                    {
                        !this.props.flags ? (
                            this.props.pseudo ? (
                                <button
                                    className={classNames({
                                        btn: true,
                                        'btn-neutral': true
                                    })}
                                    title="Custom pseudo-instructions"
                                    onClick={this._onCustom}>
                                    <i
                                        className="fa fa-code" />
                                </button>
                            ) : null
                        ) : null
                    }
                    {
                        !this.props.flags ? (
                            (!this.props.bare && !this.props.disassemble) ? (
                                <button
                                    className={classNames({
                                        btn: true,
                                        'btn-neutral': true
                                    })}
                                    title="Open in Playground"
                                    onClick={this._onOpen}>
                                    <i
                                        className="fa fa-external-link" />
                                </button>
                            ) : null
                        ) : null
                    }
                    {
                        !this.props.flags ? (
                            <button
                                className={classNames({
                                    btn: true,
                                    'btn-neutral': true
                                })}
                                title="Help"
                                data-toggle="modal"
                                data-target="#help-modal">
                                <i
                                    className="fa fa-question-circle" />
                            </button>
                        ) : null
                    }
                    <pre>
                        <code
                            className={classNames({
                                text: true,
                                show: this.state.output_show,
                                error: this.state.output_error
                            })}>{this.state.output}</code>
                    </pre>
                </div>
            </div>
        );
    }
}

export default AssemblerComponent;
