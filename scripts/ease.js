var strings = [],
    string_index = 0;

var add_string = function (str) {
    strings[string_index] = str.replace(/"/g, "");
    string_index += 1;
};

var easing_funcs = {
    easeInQuad: function (t, b, c, d) {
        var new_t = t / d;

        return c * new_t * new_t + b;
    },
    easeOutQuad: function (t, b, c, d) {
        var new_t = t / d;

        return -c * new_t * (new_t - 2) + b;
    },
    easeInOutQuad: function (t, b, c, d) {
        var new_t = t / (d / 2),
            lower_t = 0;

        if (new_t < 1) {
            return c / 2 * new_t * new_t + b;
        }

        lower_t = new_t - 1;

        return -c / 2 * (lower_t * (lower_t - 2) - 1) + b;
    },
    easeInCubic: function (t, b, c, d) {
        var new_t = t / d;

        return c * new_t * new_t * new_t + b;
    },
    easeOutCubic: function (t, b, c, d) {
        var new_t = (t / d) - 1;

        return c * (new_t * new_t * new_t + 1) + b;
    },
    easeInOutCubic: function (t, b, c, d) {
        var new_t = t / (d / 2),
            lower_t = 0;

        if (new_t < 1) {
            return c / 2 * new_t * new_t * new_t + b;
        }

        lower_t = new_t - 2;

        return c / 2 * (lower_t * lower_t * lower_t + 2) + b;
    },
    easeInQuint: function (t, b, c, d) {
        var new_t = t / d;

        return c * new_t * new_t * new_t * new_t * new_t + b;
    },
    easeOutQuint: function (t, b, c, d) {
        var new_t = (t / d) - 1;

        return c * (new_t * new_t * new_t * new_t * new_t + 1) + b;
    },
    easeInOutQuint: function (t, b, c, d) {
        var new_t = (t / (d / 2)),
            lower_t = 0;

        if (new_t < 1) {
            return c / 2 * new_t * new_t * new_t * new_t * new_t + b;
        }

        lower_t = new_t - 2;

        return c / 2 * (lower_t * lower_t * lower_t * lower_t * lower_t + 2) + b;
    },
    easeInBounce: function (t, b, c, d) {
        return c - easing_funcs.easeOutBounce(d - t, 0, c, d) + b;
    },
    easeOutBounce: function (t, b, c, d) {
        var new_t = t / d,
            lower_t = 0,
            magic = 7.5625;

        if (new_t < (1 / 2.75)) {
            return c * (magic * new_t * new_t) + b;
        } else if (new_t < (2 / 2.75)) {
            lower_t = new_t - (1.5 / 2.75);
            return c * (magic * lower_t * lower_t + 0.75) + b;
        } else if (new_t < (2.5 / 2.75)) {
            lower_t = new_t - (2.25 / 2.75);
            return c * (magic * lower_t * lower_t + 0.9375) + b;
        } else {
            lower_t = new_t - (2.625 / 2.75);
            return c * (magic * lower_t * lower_t + 0.984375) + b;
        }
    },
    easeInOutBounce: function (t, b, c, d) {
        if (t < d / 2) {
            return easing_funcs.easeInBounce(t * 2, 0, c, d) * 0.5 + b;
        }

        return easing_funcs.easeOutBounce(t * 2 - d, 0, c, d) * 0.5 + c * 0.5 + b;
    }
};

var custom = function () {
    var arg = arguments,
        start = 0,
        end = 16,
        steps = 16,
        ease = strings[0],
        func = null,
        result = [];

    if (!ease) {
        throw new Error("Missing easing type");
    }

    func = easing_funcs[ease];

    if (!func) {
        throw new Error("Invalid easing type `" + ease + "`");
    }

    if (arg[0]) {
        start = arg[0];
    }

    if (arg[1]) {
        end = arg[1];
    }

    if (arg[2]) {
        steps = arg[2];
    }

    if (steps > 255) {
        steps = 255;
    }

    for (i = 0; i < steps; i += 1) {
        value = 0;

        if (end > start) {
            value = func(i / (steps - 1), 0, 1, 1) * (end - start);
            value = Math.floor(value + start);
        } else {
            value = func((steps - i - 1) / (steps - 1), 0, 1, 1) * (start - end);
            value = Math.floor(value + end);
        }

        result.push(String.fromCharCode(value % 256));
    }

    return result.join('');
};
