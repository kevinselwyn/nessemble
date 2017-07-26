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

    addEventListener(event: string, callback: RegistryCallback): void;
    getResponseHeader(header: string): string;
    getAllResponseHeaders(): string;
    open(method: RegistryMethod, url: string, async?: boolean): void;
    send(data: any): void;
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
            text = '<raw data>...';
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

export default Registry;
