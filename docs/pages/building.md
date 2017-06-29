# Building

For guaranteed results with minimal configuration, use
[Vagrant](https://www.vagrantup.com/).

## Linux

### Dependencies

When building outside of Vagrant, run the provision script with the appropriate
flags:

```text
provision/provision.sh --home $HOME --root $ROOT
```

* `$HOME` should be the home directory (ex. `~`)
* `$ROOT` should be the root of `nessemble` (ex. `.`)

### Start

For 64-bit:

```text
vagrant up linux64 && vagrant ssh
```

For 32-bit:

```text
vagrant up linux32 && vagrant ssh
```

### Build

```text
make
```

### Package

```text
make package
```

The `.deb` will be in the `releases` directory.

## Windows

### Start

```text
vagrant up linux64 && vagrant ssh
```

### Build

For 64-bit:

```text
make win64
```

For 32-bit:

```text
make win32
```

### Package

For 64-bit:

```text
make win64_package
```

For 32-bit:

```text
make win32_package
```

The `.msi` will be in the `releases` directory.

## Mac

The only way to build for Mac is to build on a Mac.

### Dependencies

```text
brew install bison flex lua
```

### Build

```text
make
```

### Package

```text
make package
```

The `.pkg` will be in the `releases` directory.

## Javascript

### Dependencies

Install [Emscripten](http://kripken.github.io/emscripten-site/).

### Build

```text
make js
```

To minify:

```text
make min.js
```

### Test

```text
make test-js
```

## Extra

### Scripting

`nessemble` has built-in support for Javascript, Lua, and Scheme scripting.
(Read more in the section on [Extending](/extending)). These each require their
own dependencies and may be selectively turned on/off.

Pass a macro definition to `make` for each type of scripting:

```text
make SCRIPTING="-DSCRIPTING_JAVASCRIPT=0" # disable
make SCRIPTING="-DSCRIPTING_JAVASCRIPT=1" # enable (default)

make SCRIPTING="-DSCRIPTING_LUA=0" # disable
make SCRIPTING="-DSCRIPTING_LUA=1" # enable (default)

make SCRIPTING="-DSCRIPTING_SCHEME=0" # disable
make SCRIPTING="-DSCRIPTING_SCHEME=1" # enable (default)

make SCRIPTING="-DSCRIPTING_JAVASCRIPT=1 -DSCRIPTING_LUA=0 -DSCRIPTING_SCHEME=1" # combine several
```

### Servers

Several servers are included for the [Website](/), [Documentation](/), and
[Registry](/registry).

#### Website

```text
make website
```

> <div class="admonition note">
> <p class="admonition-title">Note</p>
> The website server defaults to: `http://localhost:9000`
> </div>

#### Documentation

```text
make docs
```

> <div class="admonition note">
> <p class="admonition-title">Note</p>
> The documentation server defaults to: `http://localhost:9090`
> </div>

#### Registry

```text
make registry
```

> <div class="admonition note">
> <p class="admonition-title">Note</p>
> The registry server defaults to: `http://localhost:8000`
> </div>

#### All

All 3 servers can be run as 1 server.

```text
make server
```

> <div class="admonition note">
> <p class="admonition-title">Note</p>
> The server defaults to: `http://localhost:8000`
> </div>
