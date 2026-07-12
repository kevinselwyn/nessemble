> [!WARNING]
> **This repository is deprecated.** `nessemble` has been rewritten from scratch
> in Rust as [**nessemble-rs**](https://github.com/kevinselwyn/nessemble-rs),
> which is now the actively maintained version. Please use it going forward —
> your existing `.asm` sources assemble to the same ROM output. This repository
> is kept for historical reference and will be archived.

![Nessemble](website/static/img/nessemble.gif)

`nessemble` is a 6502 assembler/disassembler/simulator targeting the Nintendo
Entertainment System

## Getting Started

To initialize a new project:

```text
nessemble init
```

Build the project:

```text
nessemble project.asm --output project.nes
```

Run `project.nes` in any NES emulator to see the result.

## Documentation

Read the [documentation](//docs.nessemble.com)
