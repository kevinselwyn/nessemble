# Nessemble Usage

```
Usage: nessemble [options] <infile.asm>

Options:
  -o, --output <outfile.rom>   output file
  -f, --format {NES,RAW}       output format
  -u, --undocumented           use undocumented opcodes
  -c, --check                  check syntax only
  -d, --disassemble            disassemble infile
  -s, --simulate <infile.rom>  start the simulator
  -r, --recipe <recipe.txt>    recipe file for the simulator
  -h, --help                   print this message
```

## Pseudo Instructions

Pseudo instructions are helpers that invoke certain functionality in the
assembler.

### .ascii

Converts a string into bytes and stores them at the current address.

Input:

```
.ascii "Hello, World!"
```

Output:

```
0000 | 48 65 6C 6C 6F 2C 20 57  6F 72 6C 64 21 | Hello, World!
```

This instruction also supports `+` and `-` to shift the value of every byte:

Input:

```
.ascii "Hello, World!"-32
```

Output:

```
0000 | 28 45 4C 4C 4F 0C 00 37  4F 52 4C 44 01 | Hello, World!
```

### .chrX

Set the CHR-ROM bank to X.

input:

```
.chr0
    .db $01, $02

.chr1
    .db $03, $04
```

Output:

```
0010 | 01 02 FF FF FF FF FF FF  FF FF FF FF FF FF FF FF |
...
2010 | 03 04 FF FF FF FF FF FF  FF FF FF FF FF FF FF FF |
...
```

### .db

Stores a single byte (or several) at the current address. Also aliased
as `.byte` and `.lobytes`.

Input:

```
.db $12, $34
```

Output:

```
0000 | 12 34 |
```

### .defchr

Define a CHR tile

For better visualization of the tile, use decimals. 0 = black, 1 = dark grey,
2 = light grey, 3 = white. Other number bases are supported, but are harder
to visualize.

Input:

```
.defchr 33333333,
        30000003,
        30000003,
        30000003,
        30000003,
        30000003,
        30000003,
        33333333
```

Output:

```
0000 | FF 81 81 81 81 81 81 FF  FF 81 81 81 81 81 81 FF |
```

### .dw

Stores a single word (or several) at the current address. Also aliased
as `.word`.

Note: words (16-bit values) are stored as `little-endian`: low byte followed by
the high byte.

Input:

```
.dw $1234, $5678
```

Output:

```
0000 | 34 12 78 56 |
```

### .hibytes

Stores the high byte of a single word (or several).

Input:

```
.hibytes $1234, $5678
```

Output:

```
0000 | 12 56 |
```
