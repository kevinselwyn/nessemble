var Nessemble = function (opts) {
    var stdout = [],
        stderr = [],
        i = 0;

    if (!window.Module) {
        return false;
    }

    return Module({
        noInitialRun: true,
        onExit: function (rc) {
            if (stdout.length && opts.onStdout) {
                opts.onStdout(rc, stdout);
            }

            if (stderr.length && opts.onStderr) {
                opts.onStderr(rc, stderr);
            }
        },
        stdin: function () {
            if (i < (opts.stdin || '').length) {
                var code = opts.stdin.charCodeAt(i);

                i += 1;

                return code;
            } else {
                return null;
            }
        },
        stdout: function (code) {
            if (code < 0) {
                code += 256;
            }

            stdout.push(code);
        },
        stderr: function (code) {
            stderr.push(code);
        }
    });
};

var NessembleExamples = function () {
    var $this = this,
        examples = document.querySelectorAll('.nessemble-example');

    if (!window.Module) {
        return;
    }

    if (!examples || !examples.length) {
        return;
    }

    [].forEach.call(examples, function (example) {
        $this.setup(example);
    });
};

NessembleExamples.prototype.addClass = function (el, className) {
    var el_classname = el.className,
        regex = new RegExp(['\\b', className, '\\b'].join(''));

    if (el_classname.search(regex) === -1) {
        el_classname += ' ' + className;
        el_classname = el_classname.replace(/\s{2,}/, ' ');
        el_classname = el_classname.replace(/^\s+/, '');
        el_classname = el_classname.replace(/\s+$/, '');

        el.className = el_classname;
    }
};

NessembleExamples.prototype.removeClass = function (el, className) {
    var el_classname = el.className,
        regex = new RegExp(['\\b', className, '\\b'].join(''));

    if (el_classname.search(regex) !== -1) {
        el_classname = el_classname.replace(regex, '');
        el_classname = el_classname.replace(/^\s+/, '');
        el_classname = el_classname.replace(/\s+$/, '');

        el.className = el_classname;
    }
};

NessembleExamples.prototype.downloadFile = function (filename, bytes, mimetype) {
    var arr = new Uint8Array(bytes),
        el = document.createElement('a');

    el.href = window.URL.createObjectURL(new Blob([arr], {
        type: mimetype
    }));
    el.download = filename;

    document.body.appendChild(el);
    el.click();
    document.body.removeChild(el);
};

NessembleExamples.prototype.dec2hex = function (dec, len) {
    var hex = dec.toString(16).toLowerCase();

    while (hex.length < len) {
        hex = '0' + hex;
    }

    return hex;
};

NessembleExamples.prototype.hexdump = function (bytes) {
    var $this = this,
        dump = [],
        index = 0,
        lines = [],
        line = {},
        chr = '',
        i = 0,
        j = 0,
        k = 0,
        l = 0;

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

    lines.forEach(function (line) {
        var str = '';

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
};

NessembleExamples.prototype.setup = function (example) {
    var $this = this,
        opts = {},
        args = [],
        rc = 0,
        stdin = example.innerHTML,
        stdout = [],
        stderr = [],
        hr = document.createElement('hr'),
        paragraph = document.createElement('p'),
        exec = document.createElement('textarea'),
        input = document.createElement('textarea'),
        buttons = document.createElement('div'),
        assemble = document.createElement('button'),
        reset = document.createElement('button'),
        clear = document.createElement('button'),
        download = document.createElement('button'),
        open = document.createElement('button'),
        help = document.createElement('button'),
        output_wrapper = document.createElement('pre'),
        output = document.createElement('code'),
        element = document.createElement('div'),
        modal_title = document.querySelector('.modal-header'),
        modal_body = document.querySelector('.modal-body');

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
    assemble.addEventListener('click', function () {
        var nessemble = new Nessemble({
            stdin: (function () {
                var str = input.value || '',
                    new_str = [],
                    bytes = [];

                if (opts.disassemble) {
                    bytes = str.split(',');

                    bytes.forEach(function (byte, i) {
                        byte = byte.replace('0x', '');
                        byte = byte.replace(/^\s+/, '');
                        byte = byte.replace(/\s+$/, '');

                        new_str.push(String.fromCharCode(parseInt(byte, 16)));
                    });

                    str = new_str.join('');
                }

                return str;
            }()),
            onStdout: function (_rc, _stdout) {
                rc = _rc;
                stdout = _stdout;

                output.innerHTML = '';
                $this.removeClass(output, 'error');

                if (!stdout.length) {
                    $this.removeClass(output, 'show');
                    download.disabled = true;
                    return;
                }

                if (opts.flags || opts.disassemble) {
                    stdout.forEach(function (byte) {
                        output.innerHTML += String.fromCharCode(byte);
                    });
                } else {
                    output.innerHTML = $this.hexdump(stdout);
                }

                $this.addClass(output, 'show');

                download.removeAttribute('disabled');
            },
            onStderr: function (_rc, _stderr) {
                rc = _rc;
                stderr = _stderr;

                output.innerHTML = '';
                $this.removeClass(output, 'error');
                download.disabled = true;

                if (!stderr.length) {
                    $this.removeClass(output, 'show');
                    return;
                }

                stderr.forEach(function (byte) {
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
    reset.addEventListener('click', function () {
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
    clear.addEventListener('click', function () {
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
    download.addEventListener('click', function () {
        $this.downloadFile('assemble.rom', stdout, 'application/octet-stream');

        ga('send', 'event', 'Assembler', 'Download', 'Documentation');
    });

    // open click listener
    open.addEventListener('click', function () {
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
};

var nessembleExamples = new NessembleExamples();
