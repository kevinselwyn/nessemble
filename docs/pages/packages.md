# Packages

`nessemble` also includes a built-in package manager. Packages can be published,
installed, and used in projects.

See the section on [Registry](/registry) to create your own registry of
`nessemble` packages.

## Search

To search for packages to install, use the `search` command:

```text
nessemble search controller
```

If a match is found, a list of packages will be returned.

## Info

To get information on a package, use the `info` command:

```text
nessemble info controller
```

If the package exists, its `README.md` file will be displayed.

## Installing

To install a package:

```text
nessemble install controller
```

Note: Packages are installed to `~/.nessemble/packages/<package-name>/` on
Linux/Mac and `\Users\<username>\.nessemble\packages\<package-name>\` on
Windows.

## Usage

To use a package in assembly:

```text
.include <controller.asm>
```

Note: `.include` with angle braces (`<...>`) will include an installed package,
and `.include` with quotes (`"..."`) includes another assembly file relative to
the project directory.

## Listing

To list all installed packages:

```text
nessemble ls
```

If packages are installed, a list of packages will be returned.

## Uninstalling

To uninstall a package:

```text
nessemble uninstall controller
```

## Publishing

To publish a package:

```text
nessemble publish lib.tar.gz
```

Note: The user must be logged in to publish a package (see the
[login](/usage/#login) command)

The following files must be in the following directory structure:

```text
lib
|-lib.asm
|-package.json
|-README.md
```

To compress:

```bash
tar -zcf lib lib.tar.gz
```

### lib.asm

`lib.asm` must include the assembly code for the package

### package.json

`package.json` must be in the following format:

```json
{
    "title": "foo",
    "description": "Foo bar baz qux",
    "version": "1.0.1",
    "author": {
        "name": "Joe Somebody",
        "email": "joe.somebody@email.com"
    },
    "license": "GPLv3",
    "tags": [
        "foo",
        "bar",
        "baz"
    ]
}
```

* `title` - String, required. Title of package.
* `description` - String, required. Description of package.
* `version` - String, required. Version of package. (Must be
    [semver](http://semver.org/) string, Ex: 1.0.1)
* `author` - Object, required. Author of package.
    * `name` - String, required. Name of author.
    * `email` - String, required. Email address of author.
* `license` - String, required. License of package.
* `tags` - Array, required. Array of tag strings.

### README.md

`README.md` must include any relevant usage information about the package.

## Updating

To update a package, publish a package zip with a `package.json` that has a
different/higher `version` than the previous.
