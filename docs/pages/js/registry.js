var Registry = function (registry, endpoint, method) {
    this.registry = registry;
    this.endpoint = endpoint;
    this.method = method;
};

Registry.prototype.run = function (callback) {
    var $this = this;

    if (this.method === 'GET') {
        this.get(function (e) {
            var xhr = this,
                headers = [
                    $this.responsePrettyStatus(xhr),
                    $this.responsePrettyHeaders(xhr.getAllResponseHeaders())
                ],
                response = $this.responsePrettyText(xhr);

            if (e.type === 'error') {
                callback('Could not reach the registry (' + $this.registry + ')', '', xhr);
            } else {
                callback(headers.join('\n'), response, xhr);
            }
        });
    } else if (this.method === 'POST') {
        this.post(function (e) {
            var xhr = this,
                headers = [
                    $this.responsePrettyStatus(xhr),
                    $this.responsePrettyHeaders(xhr.getAllResponseHeaders())
                ],
                response = $this.responsePrettyText(xhr);

            if (e.type === 'error') {
                callback('Could not reach the registry (' + $this.registry + ')', '', xhr);
            } else {
                callback(headers.join('\n'), response, xhr);
            }
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
    var header_parts = text.split('\n'),
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
    } else if (contentType === 'application/tar+gzip') {
        text = '&lt;raw data&gt;...';
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
    xhr.addEventListener('error', callback);

    xhr.open('GET', url, true);
    xhr.send(null);
};

Registry.prototype.post = function (callback) {
    var xhr = new XMLHttpRequest(),
        url = this.url();

    xhr.addEventListener('load', callback);
    xhr.addEventListener('error', callback);

    xhr.open('POST', url, true);
    xhr.send(null);
};

var RegistryExample = function () {
    var $this = this,
        examples = document.querySelectorAll('.registry-example'),
        registry_default = this.getText(document.querySelector('.registry-default')),
        registry_vals = document.querySelectorAll('.registry-default-val');

    if (!examples || !examples.length || !registry_default.length) {
        return;
    }

    this.registry = registry_default;

    [].forEach.call(registry_vals, function (registry_val) {
        registry_val.innerHTML = registry_default;
    });

    [].forEach.call(examples, function (example) {
        $this.setup(example);
    });
};

RegistryExample.prototype.getText = function (el) {
    var text = '';

    try {
        while (el.nodeType !== 3) {
            el = el.childNodes[0];
        }

        text = el.nodeValue;
    } catch (e) {
        text = '';
    }

    return text;
};

RegistryExample.prototype.addClass = function (el, className) {
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

RegistryExample.prototype.removeClass = function (el, className) {
    var el_classname = el.className,
        regex = new RegExp(['\\b', className, '\\b'].join(''));

    if (el_classname.search(regex) !== -1) {
        el_classname = el_classname.replace(regex, '');
        el_classname = el_classname.replace(/^\s+/, '');
        el_classname = el_classname.replace(/\s+$/, '');

        el.className = el_classname;
    }
};

RegistryExample.prototype.setup = function (example) {
    var $this = this,
        opts = {},
        hr = document.createElement('hr'),
        paragraph = document.createElement('p'),
        registry = document.createElement('textarea'),
        endpoint = document.createElement('textarea'),
        buttons = document.createElement('div'),
        send = document.createElement('button'),
        reset = document.createElement('button'),
        clear = document.createElement('button'),
        help = document.createElement('button'),
        output_wrapper = document.createElement('pre'),
        headers = document.createElement('code'),
        output = document.createElement('code'),
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

    // send click listener
    send.addEventListener('click', function () {
        var reg = new Registry($this.registry, endpoint.value, opts.method);

        reg.run(function (hdrs, res, xhr) {
            if (hdrs.length) {
                headers.innerHTML = hdrs;
                $this.addClass(headers, 'show');
            }

            if (res.length) {
                output.innerHTML = res;
                $this.addClass(output, 'show');
            }
        });
    });

    // endpoint keydown listener
    endpoint.addEventListener('keydown', function (e) {
        var reg = null;

        if (e.keyCode === 13) {
            e.preventDefault();

            reg = new Registry($this.registry, endpoint.value, opts.method);

            reg.run(function (hdrs, res) {
                if (hdrs.length) {
                    headers.innerHTML = hdrs;
                    $this.addClass(headers, 'show');
                }

                if (res.length) {
                    output.innerHTML = res;
                    $this.addClass(output, 'show');
                }
            });
        }
    });

    // reset click listener
    reset.addEventListener('click', function () {
        // reset endpoint
        endpoint.value = opts.endpoint;

        // clear headers
        headers.innerHTML = '';
        $this.removeClass(headers, 'show');

        // clear output
        output.innerHTML = '';
        $this.removeClass(output, 'show');
    });

    // clear click listener
    clear.addEventListener('click', function () {
        // clear headers
        headers.innerHTML = '';
        $this.removeClass(headers, 'show');

        // clear output
        output.innerHTML = '';
        $this.removeClass(output, 'show');
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
};

var registryExamples = new RegistryExample();
