declare type RegistryCallback = (event: RegistryEvent) => void;
declare type RegistryRunCallback = (hdrs: string, res: string, xhr: RegistryXHR) => void;

type RegistryMethod =
    'GET'
    | 'POST';

interface RegistryEvent {
    keyCode: number;
    preventDefault: any;
    type: string;
}

interface RegistryHeader {
    key: string;
    val: string;
}

interface RegistryXHR {
    status: number;
    statusText: string;
    responseText: string;

    addEventListener(event: string, callback: RegistryCallback);
    getResponseHeader(header: string): string;
    getAllResponseHeaders(): string;
    open(method: RegistryMethod, url: string, async?: boolean);
    send(data: any);
}

class Registry {
    registry: string;
    endpoint: string;
    method: RegistryMethod;

    constructor(registry: string, endpoint: string, method: RegistryMethod) {
        this.registry = registry;
        this.endpoint = endpoint;
        this.method = method;
    }

    run(callback: RegistryRunCallback): void {
        var $this = this;

        if (this.method === 'GET') {
            this.get(function (e: RegistryEvent): void {
                var xhr: RegistryXHR = this,
                    headers: string[] = [
                        $this.responsePrettyStatus(xhr),
                        $this.responsePrettyHeaders(xhr.getAllResponseHeaders())
                    ],
                    response: string = $this.responsePrettyText(xhr);

                if (e.type === 'error') {
                    callback(`Could not reach the registry (${$this.registry})`, '', xhr);
                } else {
                    callback(headers.join('\n'), response, xhr);
                }
            });
        } else if (this.method === 'POST') {
            this.post(function (e: RegistryEvent): void {
                var xhr: RegistryXHR = this,
                    headers: string[] = [
                        $this.responsePrettyStatus(xhr),
                        $this.responsePrettyHeaders(xhr.getAllResponseHeaders())
                    ],
                    response: string = $this.responsePrettyText(xhr);

                if (e.type === 'error') {
                    callback(`Could not reach the registry (${$this.registry})`, '', xhr);
                } else {
                    callback(headers.join('\n'), response, xhr);
                }
            });
        }
    }

    responseStatus(xhr: RegistryXHR): number {
        return xhr.status;
    }

    responseStatusText(xhr: RegistryXHR): string {
        return xhr.statusText.toUpperCase();
    }

    responsePrettyStatus(xhr: RegistryXHR): string {
        var status: number = this.responseStatus(xhr),
            statusText: string = this.responseStatusText(xhr);

        return ['HTTP/1.1', status, statusText].join(' ');
    }

    headers(text: string): RegistryHeader[] {
        var header_parts: string[] = text.split('\n'),
            headers: RegistryHeader[] = [];

        header_parts.forEach(function (part: string): void {
            var part_parts: string[] = part.split(': '),
                part_header: RegistryHeader;

            if (part_parts[0].length) {
                part_header = {
                    key: part_parts[0],
                    val: part_parts.slice(1).join(', ')
                };

                headers.push(part_header);
            }
        });

        return headers;
    }

    sortHeaders(headers: RegistryHeader[]): RegistryHeader[] {
        var sorted: RegistryHeader[] = [],
            orders: string[] = [
                'Content-Length',
                'Content-Type',
                'Access-Control-Allow-Origin',
                'Server',
                'Content-Disposition',
                'X-Response-Time',
                'X-Integrity',
                'X-RateLimit-Limit',
                'X-RateLimit-Remaining',
                'X-RateLimit-Reset',
                'Retry-After',
                'Date',
                'Cache-Control',
                'Expires'
            ],
            found: string[] = [];

        orders.forEach(function (order: string): void {
            headers.forEach(function (header: RegistryHeader): void {
                if (order === header.key) {
                    sorted.push(header);
                    found.push(order);
                }
            });
        });

        headers.forEach(function (header: RegistryHeader): void {
            if (found.indexOf(header.key) === -1) {
                sorted.push(header);
            }
        });

        return sorted;
    }

    responsePrettyHeaders(text: string): string {
        var headers: RegistryHeader[] = this.sortHeaders(this.headers(text)),
            pretty: string[] = [];

        headers.forEach(function (header: RegistryHeader): void {
            pretty.push([header.key, header.val].join(': '));
        });

        return pretty.join('\n');
    }

    responsePrettyText(xhr: RegistryXHR): string {
        var contentType: string = xhr.getResponseHeader('Content-Type'),
            data: object = {},
            text: string = '';

        if (contentType === 'application/json') {
            data = JSON.parse(xhr.responseText);
            text = JSON.stringify(data, null, 4);
        } else if (contentType === 'application/tar+gzip') {
            text = '&lt;raw data&gt;...';
        } else {
            text = xhr.responseText;
        }

        return text;
    }

    url(): string {
        var protocol: string = this.registry.match(/htt(p|ps):\/\//)[0];

        return [protocol, [this.registry.replace(protocol, ''), this.endpoint].join('/').replace(/\/+/g, '/')].join('');
    }

    get(callback: RegistryCallback): void {
        var xhr: RegistryXHR = new XMLHttpRequest(),
            url: string = this.url();

        xhr.addEventListener('load', callback);
        xhr.addEventListener('error', callback);

        xhr.open('GET', url, true);
        xhr.send(null);
    }

    post(callback: RegistryCallback): void {
        var xhr: RegistryXHR = new XMLHttpRequest(),
            url: string = this.url();

        xhr.addEventListener('load', callback);
        xhr.addEventListener('error', callback);

        xhr.open('POST', url, true);
        xhr.send(null);
    }
}

interface RegistryExampleOpts {
    method?: RegistryMethod;
    endpoint?: string;
}

class RegistryExample {
    registry: string;

    constructor() {
        var examples: NodeListOf<Node> = document.querySelectorAll('.registry-example'),
            registry_default: string = this.getText(document.querySelector('.registry-default')),
            registry_vals: NodeListOf<Node> = document.querySelectorAll('.registry-default-val');

        if (!examples || !examples.length || !registry_default.length) {
            return;
        }

        this.registry = registry_default;

        [].forEach.call(registry_vals, (registry_val: HTMLElement) => {
            registry_val.innerHTML = registry_default;
        });

        [].forEach.call(examples, (example: Node) => {
            this.setup(example);
        });
    }

    getText(el: Node): string {
        var text: string = '';

        try {
            while (el.nodeType !== 3) {
                el = el.childNodes[0];
            }

            text = el.nodeValue;
        } catch (e) {
            text = '';
        }

        return text;
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
        var el_classname = el.className,
            regex = new RegExp(['\\b', className, '\\b'].join(''));

        if (el_classname.search(regex) !== -1) {
            el_classname = el_classname.replace(regex, '');
            el_classname = el_classname.replace(/^\s+/, '');
            el_classname = el_classname.replace(/\s+$/, '');

            el.className = el_classname;
        }
    }

    setup(example) {
        var $this = this,
            opts: RegistryExampleOpts = {},
            hr: HTMLHRElement = document.createElement('hr'),
            paragraph: HTMLParagraphElement = document.createElement('p'),
            registry: HTMLTextAreaElement = document.createElement('textarea'),
            endpoint: HTMLTextAreaElement = document.createElement('textarea'),
            buttons: HTMLDivElement = document.createElement('div'),
            send: HTMLButtonElement = document.createElement('button'),
            reset: HTMLButtonElement = document.createElement('button'),
            clear: HTMLButtonElement = document.createElement('button'),
            open: HTMLButtonElement = document.createElement('button'),
            help: HTMLButtonElement = document.createElement('button'),
            output_wrapper: HTMLPreElement = document.createElement('pre'),
            headers: HTMLElement = document.createElement('code'),
            output: HTMLElement = document.createElement('code'),
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

        // send click listener
        send.addEventListener('click', function (): void {
            var reg: Registry = new Registry($this.registry, endpoint.value, opts.method);

            reg.run(function (hdrs: string, res: string, xhr: RegistryXHR): void {
                if (hdrs.length) {
                    headers.innerHTML = hdrs;
                    $this.addClass(headers, 'show');
                }

                if (res.length) {
                    output.innerHTML = res;
                    $this.addClass(output, 'show');
                }
            });

            ga('send', 'event', 'Registry', 'Send', 'Documentation', {
                dimension1: endpoint.value,
                dimension2: opts.method
            });
        });

        // endpoint keydown listener
        endpoint.addEventListener('keydown', function (e: RegistryEvent): void {
            var reg: Registry = null;

            if (e.keyCode === 13) {
                e.preventDefault();

                reg = new Registry($this.registry, endpoint.value, opts.method);

                reg.run(function (hdrs: string, res: string): void {
                    if (hdrs.length) {
                        headers.innerHTML = hdrs;
                        $this.addClass(headers, 'show');
                    }

                    if (res.length) {
                        output.innerHTML = res;
                        $this.addClass(output, 'show');
                    }
                });

                ga('send', 'event', 'Registry', 'Enter', 'Documentation', {
                    dimension1: endpoint.value,
                    dimension2: opts.method
                });
            }
        });

        // reset click listener
        reset.addEventListener('click', function (): void {
            // reset endpoint
            endpoint.value = opts.endpoint;

            // clear headers
            headers.innerHTML = '';
            $this.removeClass(headers, 'show');

            // clear output
            output.innerHTML = '';
            $this.removeClass(output, 'show');

            ga('send', 'event', 'Registry', 'Reset', 'Documentation', {
                dimension1: endpoint.value
            });
        });

        // clear click listener
        clear.addEventListener('click', function (): void {
            // clear headers
            headers.innerHTML = '';
            $this.removeClass(headers, 'show');

            // clear output
            output.innerHTML = '';
            $this.removeClass(output, 'show');

            ga('send', 'event', 'Registry', 'Clear', 'Documentation');
        });

        // open click listener
        open.addEventListener('click', function (): void {
            ga('send', 'event', 'Registry', 'Open', 'Documentation');

            window.open([$this.registry, endpoint.value].join(''), '_blank');
        });

        // clear out element
        example.innerHTML = '';

        // add paragraph
        paragraph.innerHTML = 'Try it:';

        // add registry field
        $this.addClass(registry, 'registry');
        registry.innerHTML = this.registry;
        registry.disabled = true;

        // add endpoint field
        $this.addClass(endpoint, 'endpoint');
        endpoint.innerHTML = opts.endpoint;

        // button group
        $this.addClass(buttons, 'btn-group');

        // setup send button
        $this.addClass(send, 'btn');
        $this.addClass(send, 'btn-danger');
        send.setAttribute('title', 'Send');
        send.innerHTML = 'Send';

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

        // setup open button
        if (opts.method === 'GET') {
            open.innerHTML = '<i class="fa fa-external-link"></i>';
            $this.addClass(open, 'btn');
            $this.addClass(open, 'btn-neutral');
            open.setAttribute('title', 'Open in new tab');
        } else {
            open = null;
        }

        // setup help button
        help.innerHTML = '<i class="fa fa-question-circle"></i>';
        $this.addClass(help, 'btn');
        $this.addClass(help, 'btn-neutral');
        help.setAttribute('title', 'Help');
        help.setAttribute('data-toggle', 'modal');
        help.setAttribute('data-target', '#help-modal');

        // setup help modal
        modal_title.innerHTML = 'Registry Help';
        modal_body.innerHTML = '<p>Click the <code>Send</code> button to run a specific API call.</p>';

        // setup headers
        $this.addClass(headers, 'text');
        output_wrapper.appendChild(headers);

        // setup output
        $this.addClass(output, 'text');
        output_wrapper.appendChild(output);

        // append buttons
        buttons.appendChild(send);
        buttons.appendChild(reset);
        buttons.appendChild(clear);

        if (open) {
            buttons.appendChild(open);
        }

        buttons.appendChild(help);

        // append all
        example.appendChild(hr);
        example.appendChild(paragraph);
        example.appendChild(registry);
        example.appendChild(endpoint);
        example.appendChild(buttons);
        example.appendChild(output_wrapper);

        // show module
        $this.addClass(example, 'show');

        // mark as initialized
        example.setAttribute('data-initalized', 'true');
    }
}

var registryExamples: RegistryExample = new RegistryExample();
