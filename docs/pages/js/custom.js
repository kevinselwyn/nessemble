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

(function () {
    var examples = document.querySelectorAll('.nessemble-example');

    if (!examples || !examples.length) {
        return;
    }

    [].forEach.call(examples, function (example) {
        var stdin = example.innerHTML,
            input = document.createElement('textarea'),
            assemble = document.createElement('input'),
            output_wrapper = document.createElement('pre'),
            output = document.createElement('code');

        assemble.addEventListener('click', function () {
            var nessemble = new Nessemble({
                stdin: input.value,
                onStdout: function (rc, stdout) {
                    var i = 0,
                        l = 0;

                    output.innerHTML = '';

                    for (i = 0, l = stdout.length; i < l; i += 1) {
                        output.innerHTML += '0x' + stdout[i].toString(16) + ' ';
                    }
                }
            });

            nessemble.callMain();
        });

        // clear out element
        example.innerHTML = '';

        // add stdin to textarea
        input.value = stdin;

        // setup button
        assemble.type = 'button';
        assemble.value = 'Assemble';

        // setup output
        output_wrapper.appendChild(output);

        // append all
        example.appendChild(input);
        example.appendChild(assemble);
        example.appendChild(output);
    });
}());
