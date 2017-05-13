# Disassembling

`nessemble` is not only able to assemble 6502 assembly, but also disassemble
6502 ROMs/binaries.

## Usage

```text
nessemble infile.rom --diassemble
```

## Output

### Plain ROMs

Example:

```text
0000 | 85 44    | STA <$44
0002 | 95 44    | STA <$44, X
0004 | 8D 00 44 | STA $4400
0007 | 9D 00 44 | STA $4400, X
```

The columns are as follows:

1. The offset (in bytes) in the ROM
2. The actual bytes of the instruction
3. The disassembled instruction

### iNES ROMs

For ROMs with an iNES header, there is an extra column:

```text
0010 | C000 | 85 44    | STA <$44
0012 | C002 | 95 44    | STA <$44, X
0014 | C004 | 8D 00 44 | STA $4400
0017 | C007 | 9D 00 44 | STA $4400, X
```

The columns are as follows:

1. The offset (in bytes) in the ROM
2. The offset (in bytes) in the PRG/CHR bank
3. The actual bytes of the instruction
4. The disassembled instruction

## Reassembly

To facilitate reassembly, use the `-R`/`--reassemble` flag.

```text
nessemble infile.rom --disassemble --reassemble
```

This will produce output that can be fed directly back into `nessemble` to
generate the exact same ROM that was used as input.
