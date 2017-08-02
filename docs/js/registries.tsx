import * as React from 'react';
import * as ReactDOM from 'react-dom';

import Registry from './models/registry';
import RegistryComponent from './components/registry';
import StringHelper from './helpers/string';

declare type RegistryCallback = (event: RegistryEvent) => void;

type RegistryMethod =
    'GET'
    | 'POST';

interface RegistryEvent {
    keyCode: number;
    preventDefault: any;
    type: string;
}

interface RegistryXHR {
    status: number;
    statusText: string;
    responseText: string;

    addEventListener(event: string, callback: RegistryCallback): void;
    getResponseHeader(header: string): string;
    getAllResponseHeaders(): string;
    open(method: RegistryMethod, url: string, async?: boolean): void;
    send(data: any): void;
}

interface RegistriesOpts {
    registry?: string;
    method?: RegistryMethod;
    endpoint?: string;
}

class Registries {
    constructor() {
        var examples: any = document.querySelectorAll('.registry-example'),
            registry_default: string = StringHelper.getText(document.querySelector('.registry-default')),
            registry_vals: any = document.querySelectorAll('.registry-default-val'),
            registry: string = '';

        if (!examples || !examples.length || !registry_default.length) {
            return;
        }

        registry = registry_default;

        [].forEach.call(registry_vals, (registry_val: HTMLElement) => {
            registry_val.innerHTML = registry_default;
        });

        [].forEach.call(examples, (example: HTMLElement) => {
            var opts: RegistriesOpts = {};

            try {
                opts = JSON.parse(example.getAttribute('data-opts')) || {};
            } catch (e) {
                opts = {};
            }

            opts.registry = registry;

            ReactDOM.render(<RegistryComponent {...opts} />, example);

            example.className += ' show';
        });
    }
}

var registries = new Registries();
