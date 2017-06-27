var Nessemble = function (opts) {
    var stdout = [],
        stderr = [],
        i = 0;

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

var hexdump = function (bytes) {
    var dump = [],
        dec2hex = function (dec, len) {
            var hex = dec.toString(16).toLowerCase();

            while (hex.length < len) {
                hex = '0' + hex;
            }

            return hex;
        },
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
            index: dec2hex(i, 8),
            bytes: [[], []],
            text: []
        };

        for (k = i, l = i + 8; k < l; k += 1) {
            if (k >= j) {
                break;
            }

            index += 1;
            line.bytes[0].push(dec2hex(bytes[k], 2));

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
            line.bytes[1].push(dec2hex(bytes[k], 2));

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
        index: dec2hex(index, 8),
        bytes: [[], []],
        text: []
    })

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

(function () {
    var examples = document.querySelectorAll('.nessemble-example');

    if (!examples || !examples.length) {
        return;
    }

    [].forEach.call(examples, function (example) {
        var rc = 0,
            stdin = example.innerHTML,
            stdout = [],
            stderr = [],
            opts = {},
            hr = document.createElement('hr'),
            paragraph = document.createElement('p'),
            input = document.createElement('textarea'),
            buttons = document.createElement('div'),
            assemble = document.createElement('button'),
            download = document.createElement('button'),
            output_wrapper = document.createElement('pre'),
            output = document.createElement('code'),
            element = document.createElement('div');

        // quit if already initialized
        if (example.getAttribute('data-initialized') === 'true') {
            return;
        }

        try {
            opts = JSON.parse(example.getAttribute('data-opts')) || {};
        } catch (e) {
            opts = {};
        }

        // decode html entities
        element.innerHTML = stdin;
        stdin = element.textContent;

        assemble.addEventListener('click', function () {
            var nessemble = new Nessemble({
                stdin: input.value,
                onStdout: function (_rc, _stdout) {
                    rc = _rc;
                    stdout = _stdout;

                    output.innerHTML = '';
                    output.className = output.className.replace(' error', '');

                    if (!stdout.length) {
                        output.className = output.className.replace(' show', '');
                        download.disabled = true;
                        return;
                    }

                    output.innerHTML = hexdump(stdout);

                    if (output.className.search(' show') === -1) {
                        output.className += ' show';
                    }

                    download.removeAttribute('disabled');
                },
                onStderr: function (rc, _stderr) {
                    var i = 0,
                        l = 0;

                    rc = _rc;
                    stderr = _stderr;

                    output.innerHTML = '';
                    output.className = output.className.replace(' error', '');

                    if (!stderr.length) {
                        output.className = output.className.replace(' show', '');
                        download.disabled = true;
                        return;
                    }

                    for (i = 0, l = stderr.length; i < l; i += 1) {
                        output.innerHTML += String.fromCharCode(stderr[i]);
                    }

                    if (output.className.search(' error') === -1) {
                        output.className += ' error';
                    }

                    if (output.className.search(' show') === -1) {
                        output.className += ' show';
                    }
                }
            });

            nessemble.callMain();
        });

        download.addEventListener('click', function () {
            console.log(stdout);
        });

        // clear out element
        example.innerHTML = '';

        // add paragraph
        paragraph.innerHTML = 'Try it:';

        // add stdin to textarea
        input.value = stdin;

        // button group
        buttons.className = 'btn-group';

        // setup assemble button
        assemble.innerHTML = 'Assemble';
        assemble.className = 'btn btn-danger';

        // setup download button
        download.innerHTML = 'Download';
        download.className = 'btn btn-info';
        download.disabled = true;

        // append buttons
        buttons.appendChild(assemble);

        if (opts.download) {
            buttons.appendChild(download);
        }

        // setup output
        output.className = 'text';
        output_wrapper.appendChild(output);

        // append all
        if (!opts.bare) {
            example.appendChild(hr);
            example.appendChild(paragraph);
        }

        example.appendChild(input);
        example.appendChild(buttons);
        example.appendChild(output_wrapper);

        // show module
        example.className += ' show';

        // denote bare display
        if (opts.bare) {
            example.className += ' bare';
        }

        // mark as initialized
        example.setAttribute('data-initalized', 'true');
    });
}());
