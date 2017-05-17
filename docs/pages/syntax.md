# Syntax

## Numbers

Binary, decimal, octal, hexadecimal, and ASCII character are all valid numbers.

| Base        | Example A | Example B |
|-------------|-----------|-----------|
| Binary      | %01000001 | 01000001b |
| Decimal     | 65        | 65d       |
| Octal       | 0101      | 101o      |
| Hexadecimal | $41       | 41h       |
| ASCII char  | 'A'       |           |

## Symbols

### Mathematical Operators

| Symbol   | Description |
|----------|-------------|
| +        | Add         |
| -        | Subtract    |
| \*       | Multiply    |
| /        | Divide      |
| \*\*     | Exponent    |
| &        | Bitwise AND |
| \|       | Bitwise OR  |
| ^        | Bitwise XOR |
| &gt;&gt; | Shift right |
| &lt;&lt; | Shift left  |
| %        | Modulo      |

### Comparison Operators

| Symbol | Description            |
|--------|------------------------|
| ==     | Equals                 |
| !=     | Not equals             |
| &lt;   | Less than              |
| &gt;   | Greater than           |
| &lt;=  | Less than or equals    |
| &gt;=  | Greater than or equals |

### Special

| Symbol | Description                   |
|--------|-------------------------------|
| -&gt;  | Accessor (functions like `+`) |

## Mnemonics

All 56 mnemonics are supported:

| Mnemonic | Description                          |
|----------|--------------------------------------|
| ADC      | Add with Carry                       |
| AND      | Bitwise AND with Accumulator         |
| ASL      | Arithmetic shift left                |
| BIT      | Test bits                            |
| BCC      | Branch on Carry clear                |
| BCS      | Branch on Carry set                  |
| BEQ      | Branch on equal                      |
| BMI      | Branch on minus                      |
| BNE      | Branch on not equal                  |
| BPL      | Branch on plus                       |
| BRK      | Break                                |
| BVC      | Branch on Overflow clear             |
| BVS      | Branch on Overflow set               |
| CLC      | Clear Carry                          |
| CLD      | Clear Decimal                        |
| CLI      | Clear Interrupt                      |
| CLV      | Clear Overflow                       |
| CMP      | Compare Accumulator                  |
| CPX      | Compare X register                   |
| CPY      | Compare Y register                   |
| DEC      | Decrement memory                     |
| DEX      | Decrement X register                 |
| DEY      | Decrement Y register                 |
| EOR      | Bitwise exclusive OR                 |
| INC      | Increment memory                     |
| INX      | Increment X register                 |
| INY      | Increment Y register                 |
| JMP      | Jump                                 |
| JSR      | Jump to subroutine                   |
| LDA      | Load Accumulator                     |
| LDX      | Load X register                      |
| LDY      | Load Y register                      |
| LSR      | Logical shift right                  |
| NOP      | No operation                         |
| ORA      | Bitwise OR with Accumulator          |
| PHA      | Push Accumulator                     |
| PHP      | Push processor status                |
| PLA      | Pull Accumulator                     |
| PLP      | Pull processor status                |
| ROL      | Rotate left                          |
| ROR      | Rotate right                         |
| RTI      | Return from Interrupt                |
| RTS      | Return from subroutine               |
| SBC      | Subtract with Carry                  |
| SEC      | Set Carry                            |
| SED      | Set Decimal                          |
| SEI      | Set Interrupt                        |
| STA      | Store Accumulator                    |
| STX      | Store X register                     |
| STY      | Store Y register                     |
| TAX      | Transfer Accumulator to X register   |
| TAY      | Transfer Accumulator to Y register   |
| TSX      | Transfer Stack Pointer to X register |
| TXA      | Transfer X register to Accumulator   |
| TXS      | Transfer X register to Stack Pointer |
| TYA      | Transfer Y register to Accumulator   |

Read more about 6502 opcodes
[here](http://www.6502.org/tutorials/6502opcodes.html).

In addition, 24 illegal/undocumented mnemonics may be used when assembled with
the [-u, --undocumented](/usage/#-u-undocumented) flag.

| Mnemonic | Description                                  |
|----------|----------------------------------------------|
| AAC      | AND with Accumulator                         |
| AAX      | AND X register with Accumulator              |
| ARR      | AND with Accumulator                         |
| ASR      | AND with Accumulator                         |
| ATX      | AND with Accumulator                         |
| AXA      | AND X register with Accumulator              |
| AXS      | AND X register with Accumulator              |
| DCP      | Subtract 1 from memory                       |
| DOP      | No operation (x2)                            |
| ICS      | Increase memory by 1                         |
| KIL      | Stop program counter                         |
| LAR      | AND memory with stack pointer                |
| LAX      | Load Accumulator and X register              |
| NOP      | No operation                                 |
| RLA      | Rotate one bit left in memory                |
| RRA      | Rotate one bit right in memory               |
| SBC      | Subtract with Carry                          |
| SLO      | Shift left one bit in memory                 |
| SRE      | Shift right one bit in memory                |
| SXA      | AND Y register with the high byte of address |
| SYA      | AND Y register with the high byte of address |
| TOP      | No operation (x3)                            |
| XAA      | Unknown                                      |
| XAS      | AND X register with Accumulator              |

Read more about undocumented 6502 opcodes
[here](http://nesdev.com/undocumented_opcodes.txt).

## Addressing Modes

| Mode        | Example      |
|-------------|--------------|
| Implied     | RTS          |
| Accumulator | ROL A        |
| Immediate   | LDA #$42     |
| Zeropage    | STA <$42     |
| Zeropage, X | EOR <$42, X  |
| Zeropage, Y | LDX <$42, Y  |
| Absolute    | STA $4200    |
| Absolute, X | EOR $4200, X |
| Absolute, Y | LDX $4200, Y |
| Indirect    | JMP [$4200]  |
| Indirect, X | LDA [$42, X] |
| Indirect, Y | STA [$42], Y |
| Relative    | BEQ label    |

Note: `nessemble` uses square brackets `[]` instead of parentheses `()` in its
addressing modes because the latter are used to indicate precedence in
mathematical operations.

Read more about 6502 addressing modes
[here](http://www.emulator101.com/6502-addressing-modes.html).

## Functions

| Function | Description              |
|----------|--------------------------|
| HIGH()   | Get high byte of address |
| LOW()    | Get low byte of address  |
| BANK()   | Get bank of address      |

## Pseudo-Instructions

| Pseudo-Instruction     | Description                                                                         |
|------------------------|-------------------------------------------------------------------------------------|
| [.ascii](#ascii)       | Convert ASCII string to bytes                                                       |
| [.byte](#db)           | Alias for [`.db`](#db)                                                              |
| [.checksum](#checksum) | Calculate crc32 checksum                                                            |
| [.chr](#chr)           | Set CHR bank index                                                                  |
| [.color](#color)       | Convert hex color to NES color                                                      |
| [.db](#db)             | Define 8-bit byte(s)                                                                |
| [.defchr](#defchr)     | Define CHR tile                                                                     |
| [.dw](#dw)             | Define 16-bit word(s)                                                               |
| [.else](#else)         | Else condition of an [`.if`](#if)/[`.ifdef`](#ifdef)/[`.ifndef`](#ifndef) statement |
| [.endenum](#endenum)   | End [`.enum`](#enum)                                                                |
| [.endif](#endif)       | End [`.if`](#if)/[`.ifdef`](#ifdef)/[`.ifndef`](#ifndef) statement                  |
| [.enum](#enum)         | Start enumerated variable declarations                                              |
| [.fill](#fill)         | Fill with bytes                                                                     |
| [.hibytes](#hibytes)   | Output only the high byte of 16-bit word(s)                                         |
| [.if](#if)             | Test if condition                                                                   |
| [.ifdef](#ifdef)       | Test if variable is defined                                                         |
| [.ifndef](#ifndef)     | Test if variable has not been defined                                               |
| [.incbin](#incbin)     | Include binary file                                                                 |
| [.include](#include)   | Include assembly file                                                               |
| [.incpal](#incpal)     | Include palette from PNG                                                            |
| [.incpng](#incpng)     | Include PNG                                                                         |
| [.incrle](#incrle)     | Include binary data to be RLE-encoded                                               |
| .incwav
| .ineschr
| .inesmap
| .inesmir
| .inesprg
| .inestrn
| .lobytes
| .macro
| .macrodef
| .org
| .out
| .prg
| .random
| .rsset
| .rs
| .segment
| .word

### .ascii

Convert ASCII string to bytes.

Usage:

```text
.ascii "STRING"[(+/-)NUMBER]
```

* `"STRING"` - String, required. ASCII string to turn into bytes. Must be within
quotes.
* `(+/-)NUMBER` - Number, optional. Amount to increase/decrease ASCII values.

Example:

```text
.ascii "When, in disgrace with fortune and men's eyes"
```

Output:

```text
00000000  57 68 65 6e 2c 20 69 6e  20 64 69 73 67 72 61 63  |When, in disgrac|
00000010  65 20 77 69 74 68 20 66  6f 72 74 75 6e 65 20 61  |e with fortune a|
00000020  6e 64 20 6d 65 6e 27 73  20 65 79 65 73           |nd men's eyes|
0000002d
```

The `+/-` operators may also be used to increase/decrease the output.

Example:

```text
.ascii "I all alone beweep my outcast state"-32
```

Output:

```text
00000000  29 00 41 4c 4c 00 41 4c  4f 4e 45 00 42 45 57 45  |).ALL.ALONE.BEWE|
00000010  45 50 00 4d 59 00 4f 55  54 43 41 53 54 00 53 54  |EP.MY.OUTCAST.ST|
00000020  41 54 45                                          |ATE|
00000023
```

### .checksum

Calculate crc32 checksum.

Usage:

```text
.checksum LABEL
```

* `LABEL` - Label, required. Label at which to start generating the checksum.

Example:

```text
start:
    LDA #$01
    STA <$02
    .checksum start
```

Output:

```text
00000000  a9 01 85 02 b8 1f ee 86                           |........|
00000008
```

The checksum is `b8 1f ee 86`.

Note: Checksums may only be performed on preceding data.

### .chr

Set CHR bank index.

Usage:

```text
.chr NUMBER
```

* `NUMBER` - Number, required. CHR bank index.

Example:

```text
.chr 0
```

Note: CHR banks are 2K bytes (0x2000) in size.

### .color

Convert hex color to NES color.

Finds the closest valid NES color to the given hex color.

Usage:

```text
.color NUMBER[, NUMBER, ...]
```

* `NUMBER` - Number, required. At least one number is required.
* `, NUMBER, ...` - Number(s), optional. Additional comma-separated numbers may
be used.

Example:

```text
.color $FF0000
```

Output:

```text
00000000  16                                                |.|
00000001
```

Read more about the NES color palette
[here](https://en.wikipedia.org/wiki/List_of_video_game_console_palettes#NES).

### .db

Define 8-bit byte(s).

Usage:

```text
.db NUMBER[, NUMBER, ...]
```

* `NUMBER` - Number, required. At least one number is required.
* `, NUMBER, ...` - Number(s), optional. Additional comma-separated numbers may
be used.

Example:

```text
.db $12, $34
```

Output:

```text
00000000  12 34                                             |.4|
00000002
```

### .defchr

Define CHR tile.

Only numbers from `0-3` may be used: `0` representing black, `1` dark grey, `2`
light grey, and `3` representing white.

Usage:

```text
.defchr XXXXXXXX,
        XXXXXXXX,
        XXXXXXXX,
        XXXXXXXX,
        XXXXXXXX,
        XXXXXXXX,
        XXXXXXXX,
        XXXXXXXX
```

* `XXXXXXXX,` - Number, required. Must be exactly 8 numbers of 8-characters
each.

Example:

```text
.defchr 333333333,
        300000003,
        300000003,
        300000003,
        300000003,
        300000003,
        300000003,
        333333333
```

Output:

```text
00000000  ff 01 01 01 01 01 01 ff  ff 01 01 01 01 01 01 ff  |................|
00000010
```

Read more about PPU pattern tables
[here](https://wiki.nesdev.com/w/index.php/PPU_pattern_tables).

### .dw

Define 16-bit word(s).

Usage:

```text
    .dw NUMBER[, NUMBER, ...]
```

* `NUMBER` - Number, required. At least one number is required.
* `, NUMBER, ...` - Number(s), optional. Additional comma-separated numbers may
be used.

Example:

```text
    .dw $1234, $45678
```

Output:

```text
00000000  34 12 78 56                                       |4.xV|
00000004
```

### .else

Else condition of an [`.if`](#if)/[`.ifdef`](#ifdef)/[`.ifndef`](#ifndef) statement.

Usage:

```text
.else
```

Example:

```text
.ifdef SOMETHING
    STA $00
.else
    STA $01
.endif
```

### .endenum

End [`.enum`](#enum).

Usage:

```text
.endenum
```

Example:

```text
.enum $0080

TEST_0 .rs 1
TEST_1 .rs 2
TEST_2 .rs 1

.endenum
```

### .endif

End [`.if`](#if)/[`.ifdef`](#ifdef)/[`.ifndef`](#ifndef) statement.

Usage:

```text
.endif
```

Example:

```text
.ifdef SOMETHING
    STA $00
.else
    STA $01
.endif
```

### .enum

Start enumerated variable declarations.

Usage:

```text
.enum START[, INC]
```

* `START` - Number, required. Value at which to start enumerating.
* `, INC` - Number, optional. Amount to increment after each enumeration.

Example:

```text
.enum $0080

TEST_0 .rs 1
TEST_1 .rs 2
TEST_2 .rs 1

.endenum
```

### .fill

Fill with bytes.

Usage:

```text
.fill COUNT[, VALUE]
```

* `COUNT` - Number, required. Number of bytes to fill.
* `, VALUE` - Number, optional. Value of each byte. Defaults to $FF.

Example:

```text
.fill 16
```

Output:

```text
00000000  ff ff ff ff ff ff ff ff  ff ff ff ff ff ff ff ff  |................|
00000010
```

### .hibytes

Output only the high byte of 16-bit word(s).

Usage:

```text
.hibytes NUMBER[, NUMBER]
```

* `NUMBER` - Number, required. At least one number is required.
* `, NUMBER, ...` - Number(s), optional. Additional comma-separated numbers may
be used.

Example:

```text
.hibytes $1234, $5678
```

Output:

```text
00000000  12 56                                             |.V|
00000002
```

### .if

Test if condition.

Can be accompanied by an [`.else`](#else) and must be accompanied by an
[`.endif`](#endif).

Usage:

```text
.if CONDITION
```

* `CONDITION` - Condition, required. The code that follows will be processed if
the condition is true. See [Comparison Operators](#comparison-operators).

Example:

```text
.if SOMETHING == $01
    LDA #$01
.endif
```

### .ifdef

Test if variable is defined.

Can be accompanied by an [`.else`](#else) and must be accompanied by an
[`.endif`](#endif).

Usage:

```text
.ifdef VARIABLE
```

* `VARIABLE` - Variable/constant/etc., required. The code that follows will be
processed if the variable has been defined.

Example:

```text
.ifdef SOMETHING
    STA $00
.else
    STA $01
.endif
```

### .ifndef

Test if variable has not been defined.

Can be accompanied by an [`.else`](#else) and must be accompanied by an
[`.endif`](#endif).

Usage:

```text
.ifndef VARIABLE
```

* `VARIABLE` - Variable/constant/etc., required. The code that follows will be
processed if the variable has not been defined.

Example:

```text
.ifndef SOMETHING
    STA $01
.else
    STA $00
.endif
```

### .incbin

Include binary file.

Usage:

```text
.incbin "FILENAME"[, OFFSET[, LIMIT]]
```

* `"FILENAME"` - Path to file, required. Must be within quotes.
* `[, OFFSET` - File offset index, optional. Index at which to start including
binary file.
* `[, LIMIT]]` - Limit in bytes, optional. Number of total bytes to include.

Example:

```text
.incbin "file.bin"
```

### .include

Include assembly file.

Usage:

```text
.incbin "FILENAME"
```

* `"FILENAME"` - Path to file, required. Must be within quotes.

Example:

```text
.include "file.asm"
```

Note: Included files share a global state with other included files and the main
entry point file. That means if a variable is defined in one file, it is
available to all other files, provided that they are included after the
definition.

### .incpal

Include palette from PNG.

Usage:

```text
.incpal "FILENAME"
```

* `"FILENAME"` - Path to file, required. Must be within quotes.

Example:

```text
.incpal "palette.png"
```

Note: The PNG will be scanned, row-by-row/pixel-by-pixel, from the top-left to
the bottom-right until it encounters 4 different, but not necessarily unique,
colors.

### .incpng

Include PNG.

Converts the PNG to CHR tiles. The image must include only 4 colors:

| Name       | RGB           | Hex     |
|------------|---------------|---------|
| Black      | 0, 0, 0       | #000000 |
| Dark Grey  | 85, 85, 85    | #555555 |
| Light Grey | 170, 170, 170 | #AAAAAA |
| White      | 255, 255, 255 | #FFFFFF |

Note: Other colors may be used, but accuracy is not guaranteed.

Usage:

```text
.incpng "FILENAME"
```

* `"FILENAME"` - Path to file, required. Must be within quotes.

Example:

```text
.incpng "image.png"
```

Read more about PPU pattern tables
[here](https://wiki.nesdev.com/w/index.php/PPU_pattern_tables).

### .incrle

Include binary data to be RLE-encoded

The RLE-encoding scheme used is one featured in a few Konami NES titles, known
as `Konami RLE`. The breakdown of bytes:

| Value | Description                                          |
|-------|------------------------------------------------------|
| 00-80 | Read another byte and write it to the output N times |
| 81-FE | Copy N-128 bytes from input to output                |
| FF    | End of compressed data                               |

Usage:

```text
.incrle "FILENAME"
```

* `"FILENAME"` - Path to file, required. Must be within quotes.

Read more about NES RLE compression
[here](https://wiki.nesdev.com/w/index.php/Tile_compression).

## Labels



## Macros