# Extending

`nessemble` can be extended to add custom pseudo-instructions using a number of
scripting languages.

## Usage

Use the `--pseudo` flag while compiling to point to a text file that contains
instructions for all new pseudo-instructions.

Example `pseudo.txt`:

```text
.foo = foo.lua
```

Example `example.asm`:

```text
.foo 1, 2, 3
```

To assemble:

```text
nessemble example.asm --pseudo pseudo.txt
```

## Scripting

Supported scripting languages are Lua, Scheme, Javascript, command-line scripts,
and compiled shared objects.

| Language                                  | Extension      |
|-------------------------------------------|----------------|
| [`Lua`](#lua-lua)                         | .lua           |
| [`Scheme`](#scheme-scm)                   | .scm           |
| [`Javascript`](#javascript-js)            | .js            |
| [`Scripts`](#scripts-py-sh-etc)           | .py, .sh, etc. |
| [`Shared Objects`](#shared-objects-sodll) | .so, .dll      |

### Lua (.lua)

At a minimum, the Lua script must have a function named `custom` that accepts
a variable number of integer arguments and returns a string, which will be
written directly to the ROM.

#### Example

Consider a `.foo` pseudo-instruction that multiplies integers together. It
might look something like this in Lua:

```lua
function custom(...)
    local arg={...}
    local product = 0

    if arg ~= nil and #arg > 0
    then
        for i=1,#arg
        do
            if i == 1
            then
                product = tonumber(arg[i])
            else
                product = product * tonumber(arg[i])
            end
        end
    end

    return string.char(product % 256)
end
```

#### Handling Strings

To use string arguments, an `add_string` function must be provided:

```lua
local strings = {}
local string_index = 0

function add_string(str)
    strings[string_index] = string.gsub(str, "\"", "")
    string_index = string_index + 1
end
```

> <div class="admonition note">
> <p class="admonition-title">Note</p>
> Keep in mind that strings will have quotes around them.
> </div>

Example with string arguments:

```text
.foo "product", 1, 2, 3
```

#### Handling Errors

Any errors should be thrown by calling `error()` and passing along a useful
message.

```lua
function custom(...)
    local arg={...}

    if arg == nil or #arg == 0
    then
        error("No arguments provided")
    end

    return ""
```

### Scheme (.scm)

At a minimum, the Scheme script must have a function named `custom` that accepts
a list of integers followed by a list of strings, and displays text, which will
be written directly to the ROM.

### Example

Consider a `.foo` pseudo-instruction that finds the factorial of an integer. It
might look something like this in Scheme:

```lisp
(define (factorial n)
    (if (<= n 0)
        1
            (* n (factorial (- n 1)))))

(define custom
    (lambda (ints texts)
        (display
            (factorial
                (car ints)))))
```

### Handling Strings

Strings are provided in `(list ...)` form after the list of integers.

#### Handling Errors

Any errors should be thrown by calling `(exit)` and passing along a useful
message.

Usage:

```lisp
(exit NUMBER "MESSAGE")
```

* `NUMBER` - Return code, required. Defaults to 0.
* `"MESSAGE"` - Error message, required if return code is not 0.

### Javascript (.js)

At a minimum, the Javascript script must have a function named `custom` that
accepts a variable number of integer arguments and returns a string, which will
be written directly to the ROM.

#### Example

Consider a `.foo` pseudo-instruction that finds the difference of a set of
integers. It might look something like this in Javascript:

```js
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
```

#### Handling Strings

To use string arguments, an `add_string` function must be provided:

```js
var strings = [],
    string_index = 0;

var add_string = function (str) {
    strings[string_index] = str.replace(/"/g, "");
    string_index += 1;
};
```

> <div class="admonition note">
> <p class="admonition-title">Note</p>
> Keep in mind that strings will have quotes around them.
> </div>

Example with string arguments:

```text
.foo "difference", 10, 3, 1
```

#### Handling Errors

Any errors should be thrown by calling `throw new Error()` and passing along a
useful message.

```js
var custom = function () {
    if (!arguments || !arguments.length) {
        throw new Error('No arguments provided');
    }

    return '';
};
```

### Scripts (.py, .sh, etc)

Scripts need only return a string, which will be written directly to the ROM,
and a return code, which is used to indicate an error.

#### Example

Consider a `.foo` pseudo-instruction that adds integers together. It might look
something like this in Python:

```python
#!/usr/bin/env python

import sys

def main(argc, argv):
    output = 0

    for i in range(1, argc):
        output += int(argv[i])

    sys.stdout.write(chr(output % 256))
    exit(0)

if __name__ == '__main__':
    main(len(sys.argv), sys.argv)
```

#### Handling Strings

String arguments are sent to the script before integer arguments. The argument
types will keep their precedence with other arguments of the same type, just
with all the strings appearing first.

> <div class="admonition note">
> <p class="admonition-title">Note</p>
> Keep in mind that strings will have quotes around them.
> </div>

#### Handling Errors

Any errors should be thrown by returning any non-0 return code. Any text that is
returned is assumed to be the error message:

```python
#!/usr/bin/env python

def main(argc, argv):
    arg = argv[1:]

    if len(arg) < 1:
        sys.stdout.write('No arguments provided')
        exit(1)

    exit(0)

if __name__ == '__main__':
    main(len(sys.argv), sys.argv)
```

### Shared Objects (.so/.dll)

Shared objects should include a `custom` function in the following format:

```c
int custom(char **return_str, size_t *return_len, unsigned int ints[], int length_ints, char *texts[], int length_texts)
```

* `char **return_str` - A string (char \*) passed by reference (Note: *MUST* be allocated with `malloc()` as it will be `free()`'d')
* `size_t *return_len` - Size of string passed by reference
* `unsigned int ints[]` - Array of integers from `nessemble`
* `int length_ints` - Size of `ints[]`
* `char *texts[]` - Array of strings (char \*) from `nessemble`
* `int length_texts` - Size of `*texts[]`

#### Example

Consider a `.foo` pseudo-instruction that divides integers. It might look
something like this in C:

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int custom(char **return_str, size_t *return_len, unsigned int ints[], int length_ints, char *texts[], int length_texts) {
    int arg = 0;
    unsigned int quotient = 0, i = 0, l = 0;
    char *str = NULL;

    for (i = 0, l = length_ints; i < l; i++) {
        arg = ints[i];

        if (i == 0) {
            quotient = arg;
        } else {
            quotient = (unsigned int)((double)quotient / (double)arg);
        }
    }

    str = (char *)malloc(sizeof(char) * 2);
    str[0] = quotient % 256;

    *return_str = str;
    *return_len = 1;

    return 0;
}
```

##### Compiling

```text
gcc -o quotient.so quotient.c -shared -fPIC
```

#### Handling Strings

String argument types will keep their precedence with other arguments of the
same type.

> <div class="admonition note">
> <p class="admonition-title">Note</p>
> Keep in mind that strings will have quotes around them.
> </div>

#### Handling Errors

Any errors should be thrown by returning a non-0 return code from the `custom`
function. Any text that is returned is assumed to be the error message:

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int custom(char **return_str, size_t *return_len, unsigned int ints[], int length_ints, char *texts[], int length_texts) {    
    int rc = 0;
    size_t result_len = 0;
    char *result = NULL;

    if (!length_ints && !length_texts) {
        result_len = 21;
        result = (char *)malloc(sizeof(char) * (result_len + 1));
        strcpy(result, "No arguments provided");

        rc = 1;
        goto cleanup;
    }

    result_len = 4;
    result = (char *)malloc(sizeof(char) * (result_len + 1));
    strcpy(result, "Test");

cleanup:
    *return_str = result;
    *return_len = result_len;

    return rc;
}
```
