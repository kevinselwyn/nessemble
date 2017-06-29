var Registry = function (registry, endpoint, method) {
    this.registry = registry;
    this.endpoint = endpoint;
    this.method = method;
};

Registry.prototype.run = function (callback) {
    var $this = this;

    if (this.method === 'GET') {
        this.get(function () {
            var xhr = this,
                response = [
                    $this.responsePrettyStatus(xhr),
                    $this.responsePrettyHeaders(xhr.getAllResponseHeaders()),
                    '',
                    $this.responsePrettyText(xhr)
                ];

            callback(response.join('\n'), xhr);
        });
    }
};

Registry.prototype.responseStatus = function (xhr) {
    return xhr.status;
};

Registry.prototype.responseStatusText = function (xhr) {
    return xhr.statusText.toUpperCase();
};

Registry.prototype.responsePrettyStatus = function (xhr) {
    var status = this.responseStatus(xhr),
        statusText = this.responseStatusText(xhr);

    return ['HTTP/1.1', status, statusText].join(' ');
};

Registry.prototype.headers = function (text) {
    var header_parts = text.split('\n')
        headers = [];

    header_parts.forEach(function (part) {
        var part_parts = part.split(': ');

        if (part_parts[0].length) {
            headers.push({
                key: part_parts[0],
                val: part_parts.slice(1).join(', ')
            });
        }
    });

    return headers;
};

Registry.prototype.sortHeaders = function (headers) {
    var sorted = [],
        orders = [
            'Content-Length',
            'Content-Type',
            'Access-Control-Allow-Origin',
            'Server',
            'X-Response-Time',
            'X-RateLimit-Limit',
            'X-RateLimit-Remaining',
            'X-RateLimit-Reset',
            'Retry-After',
            'Date'
        ],
        found = [];

    orders.forEach(function (order) {
        headers.forEach(function (header) {
            if (order === header.key) {
                sorted.push(header);
                found.push(order);
            }
        });
    });

    headers.forEach(function (header) {
        if (found.indexOf(header.key) === -1) {
            sorted.push(header);
        }
    });

    return sorted;
};

Registry.prototype.responsePrettyHeaders = function (text) {
    var headers = this.sortHeaders(this.headers(text)),
        pretty = [];

    headers.forEach(function (header) {
        pretty.push([header.key, header.val].join(': '));
    });

    return pretty.join('\n');
};

Registry.prototype.responsePrettyText = function (xhr) {
    var contentType = xhr.getResponseHeader('Content-Type'),
        data = null,
        text = '';

    if (contentType === 'application/json') {
        data = JSON.parse(xhr.responseText);
        text = JSON.stringify(data, null, 4);
    } else {
        text = xhr.responseText;
    }

    return text;
};

Registry.prototype.url = function () {
    var protocol = this.registry.match(/htt(p|ps):\/\//)[0];

    return [protocol, [this.registry.replace(protocol, ''), this.endpoint].join('/').replace(/\/+/g, '/')].join('');
};

Registry.prototype.get = function (callback) {
    var xhr = new XMLHttpRequest(),
        url = this.url();

    xhr.addEventListener('load', callback);

    xhr.open('GET', url, true);
    xhr.send(null);
};

var registry = new Registry('http://localhost:8000/registry', '/search/rle', 'GET');

registry.run(function (response) {
    console.log(response);
});
