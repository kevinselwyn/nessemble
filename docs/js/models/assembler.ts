declare type ModuleType = (obj: ModuleObj) => any;
declare type ModuleObjExit = (rc: number) => void;
declare type ModuleObjStdin = () => void;
declare type ModuleObjStd = (code: number) => void;

interface ModuleInterface {
    callMain?: any;
    getCustom?: any;
}

interface ModuleObj {
    noInitialRun: boolean;
    onExit: ModuleObjExit;
    stdin: ModuleObjStdin;
    stdout: ModuleObjStd;
    stderr: ModuleObjStd;
}

declare type AssemblerStd = (rc: number, data: number[]) => void;

interface AssemblerOpts {
    stdin: any;
    onStdout: AssemblerStd;
    onStderr: AssemblerStd;
    custom?: any;
}

class Assembler {
    Module: ModuleType;

    constructor(opts: AssemblerOpts) {
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

        (<ModuleInterface>this.Module).getCustom = (custom: string) => {
            if (!opts.hasOwnProperty('custom')) {
                return '';
            }

            if (opts.custom.hasOwnProperty(custom)) {
                return opts.custom[custom];
            }

            return '';
        };
    }

    callMain(args: string[]): void {
        (<ModuleInterface>this.Module).callMain(args);
    }
}

export default Assembler;
