# Using Nessemble

```
Usage: nessemble [options] <infile.asm>
                 <command> [args]

Options:
  -o, --output <outfile.rom>   output file
  -f, --format {NES,RAW}       output format
  -e, --empty <hex>            empty byte value
  -u, --undocumented           use undocumented opcodes
  -l, --list <listfile.txt>    generate list of labels and constants
  -p, --pseudo <pseudo.txt>    use custom pseudo-instruction functions
  -c, --check                  check syntax only
  -C, --coverage               log data coverage
  -d, --disassemble            disassemble infile
  -R, --reassemble             enable reassembly
  -s, --simulate <infile.rom>  start the simulator
  -r, --recipe <recipe.txt>    recipe file for the simulator
  -v, --version                display program version
  -L, --license                display program license
  -h, --help                   print this message

Commands:
  init [<arg> ...]                 initialize new project
  scripts                          install scripts
  reference [<category>] [<term>]  get reference info about assembly terms
  config [<key>] [<val>]           list/get/set config info
  registry [<url>]                 get/set registry url
  install <package>                install package
  uninstall <package>              uninstall package
  info <package>                   get info about package
  ls                               list all installed packages
  search <term>                    search for package in registry
  adduser                          create user account
  login                            login to the registry
  logout                           logout of the registry
```

## Options

### -o, --output &lt;outfile.rom&gt;

The `-o`/`--output` flag sets the filename where the output should be written.

Example:

```
nessemble infile.asm --output outfile.rom
```

If a outfile of `-` is set (or the flag is omitted altogether) the
output will be written to `stdout`.

### -f, --format {NES,RAW}

The `-f`/`--format` flag specifies the format of the output.

* `NES` - will output an NES ROM, complete with an iNES header
* `RAW` - will output raw assembled 6502 code

Example:

```
nessemble infile.asm --format RAW
```

The format is `RAW` by default, but if iNES headers are present in the assembly
code, the format will become `NES` unless this flag overrides it.

### -e, --empty &lt;hex&gt;

The `-e`/`--empty` flag sets the value of an empty byte or unfilled ROM
address value. This defaults to `FF`.

Example:

```
nessemble infile.asm --empty 00
```

### -u, --undocumented

The `-u`/`--undocumented` flag allows use of undocumented opcodes.

Example:

```
nessemble infile.asm --undocumented
```

### -l, --list &lt;listfile.txt&gt;

The `-l`/`--list` flag sets the filename where a list of labels and constants
will be written.

For disassembly, this flag sets the filename of a list to read to aid in the
disassembly process.

Example:

```
nessemble infile.asm --list listfile.txt
```

### -p, --pseudo &lt;pseudo.txt&gt;

The `-p`/`--pseudo` flag sets the filename of the file that indicates what
custom pseudo-instructions to enable.

Example:

```
nessemble infile.asm --pseudo pseudo.txt
```

### -c, --check

The `-c`/`--check` flag only checks the input assembly file for syntax errors.
Nothing will be output beyond any syntax errors that may arise.

```
nessemble infile.asm --check
```

### -C, --coverage

The `-C`/`--coverage` flag causes the assembler to output ROM coverage
information (amount that the ROM banks have data written to them).

Note: This only works when the `-f/--format` is `NES`, not when it is `RAW`.

Example:

```
nessemble infile.asm --coverage
```
