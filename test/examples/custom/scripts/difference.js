var custom = function () {
    var difference = 0,
        i = 0,
        l = 0;

    for (i = 0, l = arguments.length; i < l; i += 1) {
        if (i === 0) {
            difference = arguments[i];
        } else {
            difference -= arguments[i];
        }
    }

    return String.fromCharCode(difference % 256);
};
