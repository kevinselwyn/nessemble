import * as React from 'react';
import * as ReactDOM from 'react-dom';

import Assembler from './models/assembler';
import AssemblerComponent from './components/assembler';

interface AssemblersOpts {
    args?: string[];
    bare?: boolean;
    disassemble?: boolean;
    download?: boolean;
    flags?: string[];
    pseudo?: any;
    stdin?: string;
}

interface AssemblersHexdumpLine {
    index: string;
    bytes: string[][];
    text: string[];
}

class Assemblers {
    constructor() {
        var examples: any = document.querySelectorAll('.nessemble-assembler');

        if (!window.hasOwnProperty('Module')) {
            return;
        }

        if (!examples || !examples.length) {
            return;
        }

        [].forEach.call(examples, (example: HTMLElement) => {
            var opts: AssemblersOpts = {};

            try {
                opts = JSON.parse(example.getAttribute('data-opts')) || {};
            } catch (e) {
                opts = {};
            }

            opts.stdin = example.textContent;

            ReactDOM.render(<AssemblerComponent {...opts} />, example);

            example.className += ' show';

            if (opts.bare) {
                example.className += ' bare';
            }

            if (opts.flags) {
                example.className += ' flags';
            }
        });
    }
}

var assemblers = new Assemblers();
