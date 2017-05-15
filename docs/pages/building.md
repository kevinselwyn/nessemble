# Building

For guaranteed results with minimal configuration, use
[Vagrant](https://www.vagrantup.com/).

## Linux

### Dependencies

When building outside of Vagrant:

```text
sudo apt-get install git make curl bison flex liblua5.1-0-dev
```

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

## Javascript (experimental)

### Dependencies

Install [Emscripten](http://kripken.github.io/emscripten-site/).

### Build

```text
make js
```
