var Module = require('./lib/nessemble');

var Nessemble = function (opts) {
    var stdout = [],
        stderr = [],
        i = 0;

    opts.stdin = opts.stdin || '';

    this.Module = Module({
        noInitialRun: true,
        onExit: function (rc) {
            if (stdout.length && opts.onStdout) {
                opts.stdout(rc, stdout);
            }

            if (stderr.length && opts.onStderr) {
                opts.stderr(rc, stderr);
            }
        },
        stdin: function () {
            if (i < opts.stdin.length) {
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

    this.Module.getCustom = function (custom) {
        if (!opts.hasOwnProperty('custom')) {
            return '';
        }

        if (opts.custom.hasOwnProperty(custom)) {
            return opts.custom[custom];
        }

        return '';
    };
};

Nessemble.prototype.callMain = function (args) {
    this.Module.callMain(args);
};

if (typeof module === 'object' && module.exports) {
    module['exports'] = Nessemble;
};
