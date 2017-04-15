#!/usr/bin/env python

import re
import sys

strings = []

def easeInQuad(t, b, c, d):
    new_t = t / d

    return c * new_t * new_t + b

def easeOutQuad(t, b, c, d):
    new_t = t / d

    return -c * new_t * (new_t - 2) + b

def easeInOutQuad(t, b, c, d):
    new_t = t / (d / 2)
    lower_t = 0

    if new_t < 1:
        return c / 2 * new_t * new_t + b

    lower_t = new_t - 1

    return -c / 2 * (lower_t * (lower_t - 2) - 1) + b

def easeInCubic(t, b, c, d):
    new_t = t / d

    return c * new_t * new_t * new_t + b

def easeOutCubic(t, b, c, d):
    new_t = (t / d) - 1

    return c * (new_t * new_t * new_t + 1) + b

def easeInOutCubic(t, b, c, d):
    new_t = t / (d / 2)
    lower_t = 0

    if new_t < 1:
        return c / 2 * new_t * new_t * new_t + b

    lower_t = new_t - 2

    return c / 2 * (lower_t * lower_t * lower_t + 2) + b

def easeInQuint(t, b, c, d):
    new_t = t / d

    return c * new_t * new_t * new_t * new_t * new_t + b

def easeOutQuint(t, b, c, d):
    new_t = (t / d) - 1

    return c * (new_t * new_t * new_t * new_t * new_t + 1) + b

def easeInOutQuint(t, b, c, d):
    new_t = (t / (d / 2))
    lower_t = 0

    if new_t < 1:
        return c / 2 * new_t * new_t * new_t * new_t * new_t + b

    lower_t = new_t - 2

    return c / 2 * (lower_t * lower_t * lower_t * lower_t * lower_t + 2) + b

def easeInBounce(t, b, c, d):
    return c - easeOutBounce(d - t, 0, c, d) + b

def easeOutBounce(t, b, c, d):
    new_t = t / d
    lower_t = 0
    magic = 7.5625

    if new_t < (1 / 2.75):
        return c * (magic * new_t * new_t) + b
    elif new_t < (2 / 2.75):
        lower_t = new_t - (1.5 / 2.75)
        return c * (magic * lower_t * lower_t + 0.75) + b
    elif new_t < (2.5 / 2.75):
        lower_t = new_t - (2.25 / 2.75)
        return c * (magic * lower_t * lower_t + 0.9375) + b
    else:
        lower_t = new_t - (2.625 / 2.75)
        return c * (magic * lower_t * lower_t + 0.984375) + b

def easeInOutBounce(t, b, c, d):
    if t < d / 2:
        return easeInBounce(t * 2, 0, c, d) * 0.5 + b

    return easeOutBounce(t * 2 - d, 0, c, d) * 0.5 + c * 0.5 + b

easing_funcs = {
    'easeInQuad': easeInQuad,
    'easeOutQuad': easeOutQuad,
    'easeInOutQuad': easeInOutQuad,
    'easeInCubic': easeInCubic,
    'easeOutCubic': easeOutCubic,
    'easeInOutCubic': easeInOutCubic,
    'easeInQuint': easeInQuint,
    'easeOutQuint': easeOutQuint,
    'easeInOutQuint': easeInOutQuint,
    'easeInBounce': easeInBounce,
    'easeOutBounce': easeOutBounce,
    'easeInOutBounce': easeInOutBounce
}

def main(argc, argv):
    arg = argv[1:]
    start = 0
    end = 16
    steps = 16
    ease = None
    func = None
    result = []

    if len(arg) < 1:
        sys.stdout.write('Missing easing type')
        exit(1)

    ease = arg[0]

    if not ease in easing_funcs:
        sys.stdout.write('Invalid easing type `%s`' % (ease))
        exit(1)

    func = easing_funcs[ease]

    if len(arg) > 2:
        start = arg[1]

    if len(arg) > 3:
        end = arg[2]

    if len(arg) > 4:
        steps = arg[3]

    if steps > 255:
        steps = 255

    for i in range(0, steps):
        value = 0

        if end > start:
            value = func(i / float(steps - 1), float(0), float(1), float(1)) * (end - start)
            value += start
        else:
            value = func(float(steps - i - 1) / float(steps - 1), float(0), float(1), float(1)) * (start - end)
            value += end

        result.append(chr(int(value) % 256))

    sys.stdout.write(''.join(result))
    return 0

if __name__ == '__main__':
    main(len(sys.argv), sys.argv)
