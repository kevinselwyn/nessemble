# rle

Run-length encoding (RLE) decoder

## Explanation

`rle` utilizes the run-length encoding used in many Konami games (Blades of Steel, Contra, etc). The format is:

| Value | Meaning                                               |
|-------|-------------------------------------------------------|
| 00-80 | Read another byte, and write it to the output n times |
| 81-FE | Copy n - 128 bytes from input to output               |
| FF    | End of compressed data                                |

This library decompresses data and writes it directly to the `PPUDATA` ($2007) register. It is ideal for reading background data directly into video memory.

Registers clobbered: none

## Variables

2 bytes in the zeropage should be assigned to `rle_pointer`

## Example

```
.rsset $0000

rle_pointer .rs 2

;;;;;;;;

.include <rle.asm>

;;;;;;;;

    LDA #LOW(data)
    STA rle_pointer
    LDA #HIGH(data)
    STA rle_pointer+1

    JSR rle

;;;;;;;;

data:
    .db $04, $01, $84, $12, $34, $56, $78, $FF
```
