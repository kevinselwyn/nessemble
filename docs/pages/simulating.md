# Simulating

`nessemble` has the ability to simulate instructions in ROMs.

## Usage

```text
nessemble infile.rom --simulate
```

To provide a recipe of instructions to be run by the simulator, use the
`-R`/`--recipe` flag. See the instructions below.

```text
nessemble infile.rom --simulate --recipe recipe.txt
```

## Options

The following instructions can be used to execute functionality inside the
simulator's REPL.

| Option                                                                       | Description                                       |
|------------------------------------------------------------------------------|---------------------------------------------------|
| [.registers \[register=XXXX,...\]](#registers-registerxxxx)                  | Print registers (sets registers w/ options)       |
| [.flags \[flag=X,...\]](#flags-flagx)                                        | Print flags (sets flags w/ options)               |
| [.fill XXXX NN ...](#fill-xxxx-nn)                                           | Fill memory address with NN byte(s)               |
| [.disassemble XXXX:XXXX](#disassemble-xxxxxxxx)                              | Disassemble instructions between addresses        |
| [.instruction](#instruction)                                                 | Print next instruction                            |
| [.memory XXXX\[:XXXX\]](#memory-xxxxxxxx)                                    | Print memory in address range                     |
| [.cycles](#cycles)                                                           | Print count of CPU cycles                         |
| [.goto XXXX](#goto-xxxx)                                                     | Set program counter to XXXX                       |
| [.step \[X\]](#step-x)                                                       | Step program counter by 1 or X                    |
| [.run](#run)                                                                 | Run program                                       |
| [.record XXXX &lt;filename&gt;](#record-xxxx-filename)                       | Record writes to address XXXX                     |
| [.breakpoints](#breakpoints)                                                 | List all breakpoints                              |
| [.add_breakpoint XXXX \[&lt;name&gt;\]](#add_breakpoint-xxxx-name)           | Add breakpoint at address XXXX with optional name |
| [.remove_breakpoint \[XXXX\|&lt;name&gt;\],...](#remove_breakpoint-xxxxname) | Remove breakpoint(s) at address XXXX or with name |
| [.quit](#quit)                                                               | Quit                                              |
| [.help](#help)                                                               | Print this message                                |

### .registers [register=XXXX,...]

Prints the value of all registers. If values are provided, the registers will be
set, and then the updated value of all registers will be printed.

Example:

```text
.registers A=13,X=37
```

### .flags [flag=X,...]

Prints the value of all flags. If values are provided, the flags will be set,
and then the updated value of all flags will be printed.

> <div class="admonition note">
> <p class="admonition-title">Note</p>
> `1` means set, `0` means unset/cleared.
> </div>

Example:

```text
.flags negative=1,carry=0
```

### .fill XXXX NN ...

Fills memory with values `NN ...` starting at address `XXXX`.

Example:

```text
.fill C000 01 02 03 04
```

### .disassemble XXXX:XXXX

Disassembles and prints instructions in address range `XXXX:XXXX`.

Example:

```text
.disassemble C000:C004
```

### .instruction

Disassembles and prints the next instruction.

Example:

```text
.instruction
```

### .memory XXXX[:XXXX]

Prints memory value(s) in address range `XXXX[:XXXX]`.

Example:

```text
.memory C000
```

### .cycles

Prints the current count of CPU cycles executed.

Example:

```text
.cycles
```

### .goto XXXX

Sets the program counter to XXXX.

Example:

```text
.goto F000
```

### .step [X]

Steps the program counter by 1 or the value of `X`.

Example:

```text
.step
```

### .run

Runs the program until it hits a breakpoint or terminates.

Example:

```text
.run
```

### .record XXXX &lt;filename&gt;

Records all writes to address XXXX and writes the output to filename
`<filename>`.

Example:

```text
.record 2000 register_2000.txt
```

### .breakpoints

Prints a list of all active breakpoints.

Example:

```text
.breakpoints
```

### .add_breakpoint XXXX [&lt;name&gt;]

Adds breakpoint at address `XXXX` with optional name `<name>`.

Example:

```text
.add_breakpoint E000 stop_here
```

### .remove_breakpoint [XXXX|&lt;name&gt;,...]

Removes breakpoint(s) at address XXX or with name `<name>`.

Example:

```text
.remove_breakpoint E000,F000
```

### .quit

Quits the simulator

Example:

```text
.quit
```

### .help

Prints a help message of all simulator instructions

Example:

```text
.help
```
