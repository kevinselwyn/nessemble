# nessemble

`nessemble` is a 6502 assembler/disassembler/simulator targeting the Nintendo
Entertainment System

## Installation

First, make sure all dependencies are installed:

```
sudo apt-get -y install make bison flex
sudo apt-get -y install libpng12-dev libarchive-dev
```

To install the main executable:

```
make && sudo make install
```

## Usage

```
Usage: nessemble [options] <infile.asm>
                 <command> [args]

Options:
  -o, --output <outfile.rom>   output file
  -f, --format {NES,RAW}       output format
  -e, --empty <hex>            empty byte value
  -u, --undocumented           use undocumented opcodes
  -l, --list <listfile.txt>    generate list of labels and constants
  -c, --check                  check syntax only
  -d, --disassemble            disassemble infile
  -R, --reassemble             enable reassembly
  -s, --simulate <infile.rom>  start the simulator
  -r, --recipe <recipe.txt>    recipe file for the simulator
  -v, --version                display program version
  -L, --license                display program license
  -h, --help                   print this message

Commands:
  init                             initialize new project
  reference [<category>] [<term>]  get reference info about assembly terms
  config [<key>] [<val>]           list/get/set config info
  registry [<url>]                 get/set registry url
  install <package>                install package
  uninstall <package>              uninstall package
  info <package>                   get info about package
  ls                               list all installed packages
  search <term>                    search for package in registry
```

### Options

#### -o, --output <outfile.rom>

```
nessemble infile.asm --output outfile.rom
```

If `-o, --output` is omitted or the provided filename is `-`, output will be
written to `stdout`.

#### -f, --format {NES,RAW}

```
nessemble infile.asm --format RAW
```

Format `NES` (default) will generate a NES ROM. `RAW` will generate raw code
without an iNES header.

#### -u, --undocumented

```
nessemble infile.asm --undocumented
```

Assemble undocumented opcodes (see: Undocumented Instructions).

#### -l, --list <listfile.txt>

For assembly:

```
nessemble infile.asm --list listfile.txt
```

`listfile.txt` will contain address and values for all labels and constants.

For disassembly (with reassembly):

```
nessemble --disassemble infile.rom --reassemble --list listfile.txt
```

`infile.rom` will be disassembled with the labels added in from `listfile.txt`.

#### -c, --check

```
nessemble infile.asm --check
```

Only check for syntax errors. Does not compile.

#### -d, --disassemble

```
nessemble --disassemble infile.rom
```

Disassemble an assembled ROM.

#### -R, --reassemble

```
nessemble --disassemble infile.rom --reassemble
```

Disassemble an assembled ROM with reassembly in mind.

#### -s, --simulate <infile.rom>

```
nessemble --simulate infile.rom
```

Simulate 6502 processor.

#### -r, --recipe <recipe.txt>

```
nessemble --simulate infile.rom --recipe recipe.txt
```

Provide automated steps to run on the ROM.

#### -v, --version

```
nessemble --version
```

Outputs version information.

#### -L, --license

```
nessemble --license
```

Outputs license information.

#### -h, --help

```
nessemble --help
```

Outputs help information.

### Commands

#### init

```
nessemble init
Filename: file.asm
PRG Banks: 1
CHR Banks: 1
Mapper (0-255): 0
```

Initialize a bare project.

#### reference [<category>] [<term>]

```
nessemble reference
```

Prints all reference categories.

```
nessemble reference mappers
```

Prints all reference types for a given category.

```
nessemble reference mappers 0
```

Prints all information about given category and type.

#### config [<key>] [<val>]

```
nessemble config
```

List all config keys and values.

```
nessemble config registry
```

Get config key value.

```
nessemble config registry http://www.example.com
```

Set config key value.

#### registry [<url>]

```
nessemble registry
```

Get config registry value (alias for `config registry` option).

```
nessemble registry http://www.example.com
```

Set config registry value (alias for `config registry <val>` option).

#### install <package>

```
nessemble install controller
```

Install package.

#### uninstall <package>

```
nessemble uninstall controller
```

Uninstall package.

#### info <package>

```
nessemble info controller
```

Get info on package.

#### ls

```
nessemble ls
```

List all installed packages.

#### search <term>

```
nessemble search input
```

Search for packages that are named/tagged with the provided term.
