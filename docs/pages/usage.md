# Usage

```text
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
  publish <package.tar.gz>         publish package
  info <package>                   get info about package
  ls                               list all installed packages
  search <term>                    search for package in registry
  adduser [<arg> ...]              create user account
  login [<arg> ...]                login to the registry
  logout                           logout of the registry
  forgotpassword [<arg> ...]       send password reset email
  resetpassword [<arg> ...]        reset password
```

## Options

| Option                                                | Description                             |
|-------------------------------------------------------|-----------------------------------------|
| [-o, --output <outfile.rom>](#-o-output-outfilerom)   | output file                             |
| [-f, --format {NES,RAW}](#-f-format-nesraw)           | output format                           |
| [-e, --empty <hex>](#-e-empty-hex)                    | empty byte value                        |
| [-u, --undocumented](#-u-undocumented)                | use undocumented opcodes                |
| [-l, --list <listfile.txt>](#-l-list-listfiletxt)     | generate list of labels and constants   |
| [-p, --pseudo <pseudo.txt>](#-p-pseudo-pseudotxt)     | use custom pseudo-instruction functions |
| [-c, --check](#-c-check)                              | check syntax only                       |
| [-C, --coverage](#-c-coverage)                        | log data coverage                       |
| [-d, --disassemble](#-d-disassemble)                  | disassemble infile                      |
| [-R, --reassemble](#-r-reassemble)                    | enable reassembly                       |
| [-s, --simulate <infile.rom>](#-s-simulate-infilerom) | start the simulator                     |
| [-r, --recipe <recipe.txt>](#-r-recipe-recipetxt)     | recipe file for the simulator           |
| [-v, --version](#-v-version)                          | display program version                 |
| [-L, --license](#-l-license)                          | display program license                 |
| [-h, --help](#-h-help)                                | print this message                      |

### -o, --output &lt;outfile.rom&gt;

The `-o`/`--output` flag sets the filename where the output should be written.

Example:

```text
nessemble infile.asm --output outfile.rom
```

If a outfile of `-` is set (or the flag is omitted altogether) the
output will be written to `stdout`.

### -f, --format {NES,RAW}

The `-f`/`--format` flag specifies the format of the output.

* `NES` - will output an NES ROM, complete with an iNES header
* `RAW` - will output raw assembled 6502 code

Example:

```text
nessemble infile.asm --format RAW
```

The format is `RAW` by default, but if iNES headers are present in the assembly
code, the format will become `NES` unless this flag overrides it.

### -e, --empty &lt;hex&gt;

The `-e`/`--empty` flag sets the value of an empty byte or unfilled ROM
address value. This defaults to `FF`.

Example:

```text
nessemble infile.asm --empty 00
```

### -u, --undocumented

The `-u`/`--undocumented` flag allows use of undocumented opcodes.

Example:

```text
nessemble infile.asm --undocumented
```

### -l, --list &lt;listfile.txt&gt;

The `-l`/`--list` flag sets the filename where a list of labels and constants
will be written.

For disassembly, this flag sets the filename of a list to read to aid in the
disassembly process.

Example:

```text
nessemble infile.asm --list listfile.txt
```

### -p, --pseudo &lt;pseudo.txt&gt;

The `-p`/`--pseudo` flag sets the filename of the file that indicates what
custom pseudo-instructions to enable. See the section on
[Extending](/extending/) to learn more about how to write custom
pseudo-instructions.

Example:

```text
nessemble infile.asm --pseudo pseudo.txt
```

### -c, --check

The `-c`/`--check` flag only checks the input assembly file for syntax errors.
Nothing will be output beyond any syntax errors that may arise.

```text
nessemble infile.asm --check
```

### -C, --coverage

The `-C`/`--coverage` flag causes the assembler to output ROM coverage
information (amount that the ROM banks have data written to them).

> <div class="admonition note">
> <p class="admonition-title">Note</p>
> This only works when the `-f/--format` is `NES`, not when it is `RAW`.
> </div>

Example:

```text
nessemble infile.asm --coverage
```

### -d, --disassemble

The `-d`/`--disassemble` flag will disassemble any 6502 ROM. See the section on
[Disassembling](/disassembling/) for a breakdown of the generated output.

Example:

```text
nessemble infile.rom --diassemble
```

### -R, --reassemble

The `-R`/`--reassemble` flag can be used in conjunction with the
`-d/--diassemble` flag to disassemble to a format appropriate for reassembly.
See the section on [Disassembling](/disassembling/) for a breakdown of the
generated output.

Example:

```text
nessemble --disassemble infile.rom --reassemble
```

### -s, --simulate &lt;infile.rom&gt;

The `-s`/`--simulate` flag starts a 6502 simulator with the provided ROM. See
the section on [Simulating](/simulating/) for a breakdown of the syntax used.

Example:

```text
nessemble --simulate infile.rom
```

### -r, --recipe &lt;recipe.txt&gt;

The `-r`/`--recipe` flag can be used in conjunction with the `-s/--simulate` flag
to provide a recipe of instructions to be run by the simulator. See the section
on [Simulating](/simulating/) for a breakdown of the syntax used.

Example:

```text
nessemble --simulate infile.rom --recipe recipe.txt
```

### -v, --version

The `-v`/`--version` flag prints the version of `nessemble`.

Example:

```text
nessemble --version
```

### -L, --license

The `-L`/`--license` flag prints the license for `nessemble`. See the section on
[Licensing](/licensing/) for more information.

Example:

```text
nessemble --license
```

### -h, --help

The `-h`/`--help` flag prints the usage for `nessemble`.

Example:

```text
nessemble --help
```

## Commands

| Command                                                                     | Description                             |
|-----------------------------------------------------------------------------|-----------------------------------------|
| [init \[&lt;arg&gt; ...\]](#init-arg)                                       | initialize new project                  |
| [scripts](#scripts)                                                         | install scripts                         |
| [reference \[&lt;category&gt;\] \[&lt;term&gt;\]](#reference-category-term) | get reference info about assembly terms |
| [config \[&lt;key&gt;\] \[&lt;val&gt;\]](#config-key-val)                   | list/get/set config info                |
| [registry \[&lt;url&gt;\]](#registry-url)                                   | get/set registry url                    |
| [install &lt;package&gt;](#install-package)                                 | install package                         |
| [uninstall &lt;package&gt;](#uninstall-package)                             | uninstall package                       |
| [publish &lt;package.tar.gz&gt;](#publish-packagetargz)                     | publish package                         |
| [info &lt;package&gt;](#info-package)                                       | get info about package                  |
| [ls](#ls)                                                                   | list all installed packages             |
| [search &lt;term&gt;](#search-term)                                         | search for package in registry          |
| [adduser \[&lt;arg&gt; ...\]](#adduser-arg)                                 | create user account                     |
| [login \[&lt;arg&gt; ...\]](#login-arg)                                     | login to the registry                   |
| [logout](#logout)                                                           | logout of the registry                  |
| [forgotpassword \[&lt;arg&gt; ...\]](#forgotpassword-arg)                   | send password reset email               |
| [resetpassword \[&lt;arg&gt; ...\]](#resetpassword-arg)                     | reset password                          |

### init [&lt;arg&gt; ...]

The `init` command initializes a new project by guiding the user through the
creation of a new project.

Example:

```text
nessemble init
```

If some/no args are provided, a prompt will appear until they are all
satisfied. The order is:

```
nessemble init [filename] [prg] [chr] [mapper] [mirroring]
```

* `filename` - String. Filename of file to create.
* `prg` - Number. Number of PRG banks.
* `chr` - Number. Number of CHR banks.
* `mapper` - Number. Mapper number.
* `mirroring` - Number. Type of mirroring.

### scripts

The `scripts` command will install optional scripts that will extend the
functionality of `nessemble`. See the section on [Extending](/extending/) to
learn more about how to use custom pseudo-instructions.

Example:

```text
nessemble scripts
```

### reference [&lt;category&gt;] [&lt;term&gt;]

The `reference` command will display reference information on various aspects
of 6502 development (registers, addressing, etc).

Example:

```text
nessemble reference
```

### config [&lt;key&gt;] [&lt;val&gt;]

The `config` command will get/set configuration options. Using `config` with no
additional arguments will list all config options. Using `config` with just a
`<key>` will list the value for just that key. Using `config` with a `<key>` AND
a `<val>` will set the config key to that value.

Example:

```text
nessemble config
```

### registry [&lt;url&gt;]

The `registry` command will get/set the registry URL. Using `registry` with no
additional arguments will list the current registry URL. Using `registry` with
a URL will set the registry URL to that value.

> <div class="admonition note">
> <p class="admonition-title">Note</p>
> This only works when the `-f/--format` is `NES`, not when it is `RAW`.
> </div>

Example:

```text
nessemble registry
```

### install &lt;package&gt;

The `install` command will install a package that can be used in assembly
projects. See the section on [Packages](/packages/#installing) for more
information on the usage.

Example:

```text
nessemble install foo
```

### uninstall &lt;package&gt;

The `uninstall` command will uninstall a package. See the section on
[Packages](/packages/#uninstalling) for more information on the usage.

Example:

```text
nessemble uninstall foo
```

### publish &lt;package.tar.gz&gt;

The `publish` command will publish a package. See the section on
[Packages](/packages/#publishing) for more information on the usage.

Example:

```text
nessemble publish foo.tar.gz
```

### info &lt;package&gt;

The `info` command will print info for a given package. See the section on
[Packages](/packages/#info) for more information on the usage.

Example:

```text
nessemble info foo
```

### ls

The `ls` command will list all installed packages. See the section on
[Packages](/packages/#listing) for more information on the usage.

Example:

```text
nessemble ls
```

### search &lt;term&gt;

The `search` command will search for packages with titles, descriptions, or
keywords that match the given term. See the section on
[Packages](/packages/#searching) for more information on the usage.

Example:

```text
nessemble search foo
```

### adduser [&lt;arg&gt; ...]

The `adduser` command will guide the user in the creation of a new user in the
registry.

Example:

```text
nessemble adduser
```

If some/no args are provided, a prompt will appear until they are all
satisfied. The order is:

```text
nessemble adduser [name] [username] [email]
```

* `name` - String. Name of user.
* `username` - String. Proposed username.
* `email` - String. Email address of user.

> <div class="admonition note">
> <p class="admonition-title">Note</p>
> For added security, a `password` will always be gathered from the user by
> prompt.
> </div>

### login [&lt;arg&gt; ...]

The `login` command will guide the user in the registry login process.

Example:

```text
nessemble login
```

If some/no args are provided, a prompt will appear until they are all
satisfied. The order is:

```text
nessemble login [username]
```

* `username` - String. User's username.

> <div class="admonition note">
> <p class="admonition-title">Note</p>
> For added security, a `password` will always be gathered from the user by
> prompt.
> </div>

### logout

The `logout` command will log the user out of the registry.

Example:

```text
nessemble logout
```

### forgotpassword [&lt;arg&gt; ...]

The `forgotpassword` command will send a password reset email to the user.

Example:

```text
nessemble forgotpassword
```

If some/no args are provided, a prompt will appear until they are all
satisfied. The order is:

```text
nessemble forgotpassword [username]
```

* `username` - String. User's username.

### resetpassword [&lt;arg&gt; ...]

The `resetpassword` command will prompt the user to reset their password.

Example:

```text
nessemble resetpassword
```

If some/no args are provided, a prompt will appear until they are all
satisfied. The order is:

```text
nessemble resetpassword [username]
```

* `token` - String. Reset token.
* `username` - String. User's username.

> <div class="admonition note">
> <p class="admonition-title">Note</p>
> For added security, a `password` will always be gathered from the user by
> prompt.
> </div>
