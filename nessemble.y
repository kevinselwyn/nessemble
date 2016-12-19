%{

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdarg.h>
#include "nessemble.h"
#include "opcodes.h"

// bison
int yylineno;
int yylex();
int yyparse();
FILE *yyin;

// path
char *cwd_path = NULL;

// cli
int flags = 0;

// include
int include_stack_ptr;

// io
unsigned int *rom = NULL;
int pass = 1;
int offset_max = 0;
int bank_index = 0;
int bank_offsets[MAX_BANKS];

// banks
int prg_offsets[MAX_BANKS];
int chr_offsets[MAX_BANKS];
int prg_index = 0;
int chr_index = 0;

// segment
char *segment = NULL;
int segment_type = 0;

// trainer
unsigned int trainer[TRAINER_MAX];
int offset_trainer = 0;

// symbols
int symbol_index = 0;
int rsset = 0;
int get_symbol(char *name);

// input
unsigned int length_ints = 0;
unsigned int ints[MAX_INTS];

// ines
struct ines_header ines = { 1, 0, 0, 1, 0 };

%}

%union {
    char *sval;
    char cval;
    int ival;
}

%token ENDL
%token INDENTATION

%token PLUS
%token MINUS
%token MULT
%token DIV

%token HASH
%token COMMA
%token EQU
%token COLON
%token OPEN_PAREN
%token CLOSE_PAREN
%token OPEN_BRACK
%token CLOSE_BRACK
%token CARET

%token HIGH
%token LOW

%token PSEUDO_ASCII
%token PSEUDO_BANK
%token PSEUDO_BYTE
%token PSEUDO_DB
%token PSEUDO_DEFCHR
%token PSEUDO_DW
%token PSEUDO_HIBYTES
%token PSEUDO_INCBIN
%token PSEUDO_INCLUDE
%token PSEUDO_INCPNG
%token PSEUDO_INESCHR
%token PSEUDO_INESMAP
%token PSEUDO_INESMIR
%token PSEUDO_INESPRG
%token PSEUDO_INESTRN
%token PSEUDO_LOBYTES
%token PSEUDO_ORG
%token PSEUDO_OUT
%token PSEUDO_RSSET
%token PSEUDO_RS
%token PSEUDO_SEGMENT
%token PSEUDO_WORD

%token <ival> NUMBER_HEX
%token <ival> NUMBER_BIN
%token <ival> NUMBER_OCT
%token <ival> NUMBER_DEC
%token <ival> PSEUDO_CHR
%token <ival> PSEUDO_PRG

%token <sval> TEXT
%token <sval> QUOT_STRING

%token <cval> CHAR_A
%token <cval> CHAR_X
%token <cval> CHAR_Y

%token FEOF
%token UNKNOWN

%type <sval> pseudo_ascii
%type <sval> pseudo_incbin
%type <sval> pseudo_include
%type <sval> pseudo_incpng
%type <sval> pseudo_inestrn
%type <sval> pseudo_out
%type <sval> pseudo_segment

%type <ival> number
%type <ival> number_base
%type <ival> number_highlow
%type <ival> label
%type <ival> pseudo_bank
%type <ival> pseudo_chr
%type <ival> pseudo_ineschr
%type <ival> pseudo_inesmap
%type <ival> pseudo_inesmir
%type <ival> pseudo_inesprg
%type <ival> pseudo_org
%type <ival> pseudo_prg
%type <ival> pseudo_rsset

%%

/* Program */

program
    : line         { /* NOTHING */ }
    | program line { /* NOTHING */ }
    ;

/* Atomic */

open_char
    : OPEN_PAREN
    | OPEN_BRACK
    ;

close_char
    : CLOSE_PAREN
    | CLOSE_BRACK
    ;

number
    : number_base              { $$ = $1; }
    | number PLUS number_base  { $$ = $1 + $3; }
    | number MINUS number_base { $$ = $1 - $3; }
    ;

number_base
    : NUMBER_HEX     { $$ = $1; }
    | NUMBER_BIN     { $$ = $1; }
    | NUMBER_OCT     { $$ = $1; }
    | NUMBER_DEC     { $$ = $1; }
    | number_highlow { $$ = $1; }
    ;

number_highlow
    : LOW open_char number_base close_char  { $$ = $3 & 255; }
    | LOW open_char label close_char        { $$ = $3 & 255; }
    | HIGH open_char number_base close_char { $$ = ($3 >> 8) & 255; }
    | HIGH open_char label close_char       { $$ = ($3 >> 8) & 255; }
    ;

label
    : TEXT                    { if (pass == 2) { $$ = symbols[get_symbol($1)].value; } else { $$ = 0; } }
    | label PLUS number_base  { if (pass == 2) { $$ = $1 + $3; } else { $$ = 0; } }
    | label MINUS number_base { if (pass == 2) { $$ = $1 - $3; } else { $$ = 0; } }
    ;

comma
    : COMMA                  { /* NOTHING */ }
    | COMMA ENDL             { /* NOTHING */ }
    | COMMA ENDL INDENTATION { /* NOTHING */ }
    ;

/* Lines */

line
    : ENDL                    { /* NOTHING */ }
    | FEOF                    { include_file_pop(); if (include_stack_ptr < 0) { YYACCEPT; } }
    | INDENTATION             { /* NOTHING */ }
    | pseudo                  { /* NOTHING */ }
    | constant_decl           { /* NOTHING */ }
    | label_decl              { /* NOTHING */ }
    | INDENTATION instruction { /* NOTHING */ }
    ;

/* Pseudo */

pseudo
    : pseudo_ascii   { /* NOTHING */ }
    | pseudo_bank    { pseudo_bank($1); }
    | pseudo_chr     { pseudo_chr($1); }
    | pseudo_db      { pseudo_db(); }
    | pseudo_defchr  { pseudo_defchr(); }
    | pseudo_dw      { pseudo_dw(); }
    | pseudo_hibytes { pseudo_hibytes(); }
    | pseudo_incbin  { /* NOTHING */ }
    | pseudo_include { pseudo_include($1); }
    | pseudo_incpng  { /* NOTHING */ }
    | pseudo_ineschr { pseudo_ineschr($1); }
    | pseudo_inesmap { pseudo_inesmap($1); }
    | pseudo_inesmir { pseudo_inesmir($1); }
    | pseudo_inesprg { pseudo_inesprg($1); }
    | pseudo_inestrn { pseudo_inestrn($1); }
    | pseudo_lobytes { pseudo_lobytes(); }
    | pseudo_org     { pseudo_org($1); }
    | pseudo_out     { pseudo_out($1); }
    | pseudo_prg     { pseudo_prg($1); }
    | pseudo_rsset   { pseudo_rsset($1); }
    | pseudo_rs      { /* NOTHING */ }
    | pseudo_segment { pseudo_segment($1); }
    ;

pseudo_ascii
    : PSEUDO_ASCII QUOT_STRING                   { pseudo_ascii($2, 0); }
    | PSEUDO_ASCII QUOT_STRING PLUS number_base  { pseudo_ascii($2, $4); }
    | PSEUDO_ASCII QUOT_STRING MINUS number_base { pseudo_ascii($2, $4 * -1); }
    ;

pseudo_bank
    : PSEUDO_BANK number { $$ = $2; }
    | PSEUDO_BANK label  { $$ = $2; }
    ;

pseudo_chr
    : PSEUDO_CHR { $$ = $1; }
    ;

pseudo_db_alias
    : PSEUDO_DB   { /* NOTHING */ }
    | PSEUDO_BYTE { /* NOTHING */ }
    ;

pseudo_db
    : pseudo_db_alias number { ints[length_ints++] = $2; }
    | pseudo_db_alias label  { ints[length_ints++] = $2; }
    | pseudo_db COMMA number { ints[length_ints++] = $3; }
    | pseudo_db COMMA label  { ints[length_ints++] = $3; }
    ;

pseudo_defchr
    : PSEUDO_DEFCHR number       { ints[length_ints++] = $2; }
    | pseudo_defchr comma number { ints[length_ints++] = $3; }
    ;

pseudo_dw_alias
    : PSEUDO_DW   { /* NOTHING */ }
    | PSEUDO_WORD { /* NOTHING */ }
    ;

pseudo_dw
    : pseudo_dw_alias number { ints[length_ints++] = $2; }
    | pseudo_dw_alias label  { ints[length_ints++] = $2; }
    | pseudo_dw COMMA number { ints[length_ints++] = $3; }
    | pseudo_dw COMMA label  { ints[length_ints++] = $3; }
    ;

pseudo_hibytes
    : PSEUDO_HIBYTES number       { ints[length_ints++] = $2; }
    | PSEUDO_HIBYTES label        { ints[length_ints++] = $2; }
    | pseudo_hibytes COMMA number { ints[length_ints++] = $3; }
    | pseudo_hibytes COMMA label  { ints[length_ints++] = $3; }
    ;

pseudo_incbin
    : PSEUDO_INCBIN QUOT_STRING                           { pseudo_incbin($2, 0, -1); }
    | PSEUDO_INCBIN QUOT_STRING COMMA number              { pseudo_incbin($2, $4, -1); }
    | PSEUDO_INCBIN QUOT_STRING COMMA number COMMA number { pseudo_incbin($2, $4, $6); }
    ;

pseudo_include
    : PSEUDO_INCLUDE QUOT_STRING ENDL { $$ = $2; }
    ;

pseudo_incpng
    : PSEUDO_INCPNG QUOT_STRING                           { pseudo_incpng($2, 0, -1); }
    | PSEUDO_INCPNG QUOT_STRING COMMA number              { pseudo_incpng($2, $4, -1); }
    | PSEUDO_INCPNG QUOT_STRING COMMA number COMMA number { pseudo_incpng($2, $4, $6); }
    ;

pseudo_ineschr
    : PSEUDO_INESCHR number { $$ = $2; }
    | PSEUDO_INESCHR label  { $$ = $2; }
    ;

pseudo_inesmap
    : PSEUDO_INESMAP number { $$ = $2; }
    | PSEUDO_INESMAP label  { $$ = $2; }
    ;

pseudo_inesmir
    : PSEUDO_INESMIR number { $$ = $2; }
    | PSEUDO_INESMIR label  { $$ = $2; }
    ;

pseudo_inesprg
    : PSEUDO_INESPRG number { $$ = $2; }
    | PSEUDO_INESPRG label  { $$ = $2; }
    ;

pseudo_inestrn
    : PSEUDO_INESTRN QUOT_STRING { $$ = $2; }
    ;

pseudo_lobytes
    : PSEUDO_LOBYTES number       { ints[length_ints++] = $2; }
    | PSEUDO_LOBYTES label        { ints[length_ints++] = $2; }
    | pseudo_lobytes COMMA number { ints[length_ints++] = $3; }
    | pseudo_lobytes COMMA label  { ints[length_ints++] = $3; }
    ;

pseudo_org
    : PSEUDO_ORG number { $$ = $2; }
    | PSEUDO_ORG label  { $$ = $2; }
    ;

pseudo_out
    : PSEUDO_OUT QUOT_STRING { $$ = $2; }
    ;

pseudo_prg
    : PSEUDO_PRG { $$ = $1; }
    ;

pseudo_rsset
    : PSEUDO_RSSET number { $$ = $2; }
    | PSEUDO_RSSET label  { $$ = $2; }
    ;

pseudo_rs
    : TEXT PSEUDO_RS number { pseudo_rs($1, $3); }
    | TEXT PSEUDO_RS label  { pseudo_rs($1, $3); }
    ;

pseudo_segment
    : PSEUDO_SEGMENT QUOT_STRING { $$ = $2; }
    ;

/* Constant */

constant_decl
    : TEXT EQU number { add_constant($1, $3); }
    ;

/* Label */

label_decl
    : TEXT COLON { add_label($1); }
    ;

/* Instruction */

instruction
    : TEXT CHAR_A                                   { assemble_accumulator($1); }
    | TEXT HASH number                              { assemble_immediate($1, $3); }
    | TEXT HASH label                               { assemble_immediate($1, $3); }
    | TEXT open_char number close_char              { assemble_indirect($1, $3); }
    | TEXT open_char label close_char               { assemble_indirect($1, $3); }
    | TEXT open_char number COMMA CHAR_X close_char { assemble_indirect_xy($1, $3, $5); }
    | TEXT open_char label COMMA CHAR_X close_char  { assemble_indirect_xy($1, $3, $5); }
    | TEXT open_char number close_char COMMA CHAR_Y { assemble_indirect_xy($1, $3, $6); }
    | TEXT open_char label close_char COMMA CHAR_Y  { assemble_indirect_xy($1, $3, $6); }
    | TEXT CARET number COMMA CHAR_X                { assemble_zeropage_xy($1, $3, $5); }
    | TEXT CARET label COMMA CHAR_X                 { assemble_zeropage_xy($1, $3, $5); }
    | TEXT CARET number COMMA CHAR_Y                { assemble_zeropage_xy($1, $3, $5); }
    | TEXT CARET label COMMA CHAR_Y                 { assemble_zeropage_xy($1, $3, $5); }
    | TEXT number COMMA CHAR_X                      { assemble_absolute_xy($1, $2, $4); }
    | TEXT label COMMA CHAR_X                       { assemble_absolute_xy($1, $2, $4); }
    | TEXT number COMMA CHAR_Y                      { assemble_absolute_xy($1, $2, $4); }
    | TEXT label COMMA CHAR_Y                       { assemble_absolute_xy($1, $2, $4); }
    | TEXT CARET number ENDL                        { assemble_zeropage($1, $3); }
    | TEXT CARET label ENDL                         { assemble_zeropage($1, $3); }
    | TEXT number ENDL                              { assemble_absolute($1, $2); }
    | TEXT label ENDL                               { assemble_absolute($1, $2); }
    | TEXT ENDL                                     { assemble_implied($1); }
    ;

%%

/**
 * Program usage
 * @param {string *} exec - Executable name
 * @param {int} Return code
 */
int usage(char *exec) {
    printf("Usage: %s [options] <infile.asm>\n\n", exec);
    printf("Options:\n");
    printf("  -o, --output <outfile.rom>   output file\n");
    printf("  -f, --format (NES|RAW)       output format\n");
    printf("  -u, --undocumented           use undocumented opcodes\n");
    printf("  -h, --help                   print this message\n");

    return RETURN_USAGE;
}

/**
 * Main function
 * @param {int} argc - Argument count
 * @param {char *} argv[] - Argument array
 */
int main(int argc, char *argv[]) {
    int rc = RETURN_OK, i = 0, l = 0, byte = 0;
    char *exec = NULL, *filename = NULL, *outfilename = NULL;
    FILE *file = NULL, *outfile = NULL;

    // exec
    exec = argv[0];

    // usage
    if (argc < 2) {
        rc = usage(exec);
        goto cleanup;
    }

    // parse args
    for (i = 1, l = argc; i < l; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            rc = usage(exec);
            goto cleanup;
        }

        if (strcmp(argv[i], "-u") == 0 || strcmp(argv[i], "--undocumented") == 0) {
            flags |= FLAG_UNDOCUMENTED;
            continue;
        }

        if (strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--output") == 0) {
            if (i + 1 < l) {
                outfilename = argv[i + 1];
            } else {
                rc = usage(exec);
                goto cleanup;
            }

            i += 1;
            continue;
        }

        if (strcmp(argv[i], "-f") == 0 || strcmp(argv[i], "--format") == 0) {
            if (i + 1 < l) {
                if (strcmp(argv[i+1], "nes") == 0 || strcmp(argv[i+1], "NES") == 0) {
                    flags |= FLAG_NES;
                }
            } else {
                rc = usage(exec);
                goto cleanup;
            }

            i += 1;
            continue;
        }

        filename = argv[i];
    }

    // get cwd
    realpath(filename, cwd);
    cwd_path = strdup(cwd);

    for (i = (int)strlen(cwd_path), l = 0; i >= l; i--) {
        if (cwd_path[i] == '/') {
            cwd_path[i] = '\0';
            break;
        }
    }

    // open file
    file = fopen(cwd, "r");

    if (!file) {
        fprintf(stderr, "Could not open %s\n", filename);

        rc = RETURN_EPERM;
        goto cleanup;
    }

    yyin = file;

    // segment
    segment = (char *)malloc(sizeof(char) * 8);

    /* PASS 1 */

    pass = 1;

    do {
        yyparse();
    } while (!feof(yyin) && pass == 1);

    /* PASS 2 */

    // restart
    fseek(yyin, 0, 0);

    // get offset max
    if (is_flag_nes()) {
        offset_max = (ines.prg * BANK_PRG) + (ines.chr * BANK_CHR);
    }

    // create rom
    rom = (unsigned int *)malloc(sizeof(unsigned int) * offset_max);

    if (!rom) {
        fprintf(stderr, "Memory error\n");

        rc = RETURN_EPERM;
        goto cleanup;
    }

    // set all bytes to 0xFF
    for (i = 0, l = offset_max; i < l; i++) {
        rom[i] = 0xFF;
    }

    // set all trainer bytes to 0xFF
    for (i = 0, l = TRAINER_MAX; i < l; i++) {
        trainer[i] = 0xFF;
    }

    // clear offsets
    offset_trainer = 0;
    bank_index = 0;

    for (i = 0, l = MAX_BANKS; i < l; i++) {
        bank_offsets[i] = 0x00;
    }

    // reset lineno
    yylineno = 1;

    do {
        yyparse();
    } while (!feof(yyin) && pass == 2);

    /* DONE */

    // output
    if (!outfilename || strcmp(outfilename, "-") == 0) {
        outfilename = "/dev/stdout";
    }

    // write output
    outfile = fopen(outfilename, "w");

    // write nes header
    if (is_flag_nes()) {
        fwrite("NES", 3, 1, outfile); // 0-2

        byte = 0x1A;
        fwrite(&byte, 1, 1, outfile); // 3

        fwrite(&ines.prg, 1, 1, outfile); // 4
        fwrite(&ines.chr, 1, 1, outfile); // 5

        byte = 0;
        byte |= ines.mir & 0x01;
        byte |= (ines.trn & 0x01) << 0x02;
        byte |= (ines.map & 0x0F) << 0x04;

        fwrite(&byte, 1, 1, outfile); // 6

        byte = 0;
        byte |= (ines.map & 0xF0);

        fwrite(&byte, 1, 1, outfile); // 7

        byte = 0;

        fwrite(&byte, 1, 1, outfile); // 8
        fwrite(&byte, 1, 1, outfile); // 9
        fwrite(&byte, 1, 1, outfile); // 10

        for (i = 11, l = 16; i < l; i++) {
            fwrite(&byte, 1, 1, outfile); // 11-15
        }
    }

    // write trainer
    if (ines.trn == 1) {
        for (i = 0, l = TRAINER_MAX; i < l; i++) {
            fwrite(trainer+i, 1, 1, outfile);
        }
    }

    // write rom data
    for (i = 0, l = offset_max; i < l; i++) {
        fwrite(rom+i, 1, 1, outfile);
    }

cleanup:
    if (file) {
        fclose(file);
    }

    return rc;
}

/**
 * Test if undocumented flag is active
 * @return {int} True if flag active, false if not
 */
int is_flag_undocumented() {
    return (flags & FLAG_UNDOCUMENTED) != 0;
}

/**
 * Test if nes flag is active
 * @return {int} True if flag active, false if not
 */
int is_flag_nes() {
    return (flags & FLAG_NES) != 0;
}

/**
 * Test if CHR segment
 * @return {int} True if is segment, false if not
 */
int is_segment_chr() {
    return segment_type == SEGMENT_CHR;
}

/**
 * Test if PRG segment
 * @return {int} True if is segment, false if not
 */
int is_segment_prg() {
    return segment_type == SEGMENT_PRG;
}

/**
 * Parser error
 * @param {const char *} fmt - Format string
 * @param {...} ... - Variable arguments
 */
void yyerror(const char *fmt, ...) {
    int rc = RETURN_EPERM;

    va_list argptr;
    va_start(argptr, fmt);

    fprintf(stderr, "Error on line %d: ", yylineno);
    vfprintf(stderr, fmt, argptr);
    fprintf(stderr, "\n");

    va_end(argptr);

    exit(rc);
}

/**
 * Handle EOF
 */
void assemble_feof() {
    if (--include_stack_ptr >= 0) {
        include_file_pop();
    }
}

/**
 * End parser pass
 */
void end_pass() {
    pass++;
}

/**
 * Get ROM index
 * @return {int} ROM index
 */
int get_rom_index() {
    int index = 0;

    if (bank_index < ines.prg) {
        index = bank_index * BANK_PRG;
    } else {
        index = (ines.prg * BANK_PRG) + ((bank_index - ines.prg) * BANK_CHR);
    }

    index += bank_offsets[bank_index];

    return index;
}

/**
 * Get address offset
 * @return {int} PC offset
 */
int get_address_offset() {
    if (is_segment_prg()) {

    }

    if (is_segment_chr()) {

    }

    return bank_offsets[bank_index];
}

/**
 * Write byte
 * @param {unsigned int} byte - Byte to write
 */
void write_byte(unsigned int byte) {
    int offset = get_rom_index();

    if (ines.trn == 1) {
        if (pass == 2) {
            trainer[offset_trainer] = byte;
        }

        offset_trainer++;

        return;
    }

    if (pass == 2) {
        rom[offset] = byte;
    }

    bank_offsets[bank_index]++;

    if (offset + 1 > offset_max) {
        offset_max = offset + 1;
    }
}

/**
 * Add symbol
 * @param {char *} name - Symbol name
 * @param {int} value - Symbol value
 * @param {int} type - Symbol type
 */
void add_symbol(char *name, int value, int type) {
    if (pass == 1) {
        symbols[symbol_index].name = name;
        symbols[symbol_index].value = value;
        symbols[symbol_index++].type = type;
    }
}

/**
 * Get symbol
 * @param {char *} name - Symbol name
 * @return {int} Symbol ID
 */
int get_symbol(char *name) {
    int i = 0, l = 0, symbol_id = -1;

    for (i = 0, l = symbol_index; i < l; i++) {
        if (symbol_id == -1 && strcmp(symbols[i].name, name) == 0) {
            symbol_id = i;
        }
    }

    if (symbol_id == -1) {
        yyerror("Invalid symbol `%s`", name);
    }

    return symbol_id;
}

/**
 * Add constant
 * @param {char *} name - Constant name
 * @param {int} value - Constant value
 */
void add_constant(char *name, int value) {
    add_symbol(name, value, SYMBOL_CONSTANT);
}

/**
 * Add label
 * @param {char *} name - Label name
 */
void add_label(char *name) {
    int offset = get_address_offset();

    add_symbol(name, offset, SYMBOL_LABEL);
}

/**
 * .ascii pseudo instruction
 * @param {char *} string - ASCII string
 * @param {int} offset - Offset
 */
void pseudo_ascii(char *string, int offset) {
    int i = 0, l = 0;
    size_t length = 0;

    length = strlen(string);

    for (i = 1, l = (int)length - 1; i < l; i++) {
        write_byte((unsigned int)string[i] + offset);
    }
}

/**
 * .bank pseudo instruction
 * @param {int} value - Bank value
 */
void pseudo_bank(int value) {
    flags |= FLAG_NES;

    // TODO: add check for too high bank

    bank_index = value;
}

/**
 * .chr[0-9]+ pseudo instruction
 * @param {int} index - CHR bank index
 */
void pseudo_chr(int index) {
    segment_type = SEGMENT_CHR;

    chr_index = index;
}

/**
 * .db pseudo instruction
 */
void pseudo_db() {
    int i = 0, l = 0;

    for (i = 0, l = length_ints; i < l; i++) {
        write_byte(ints[i] & 0xFF);
    }

    length_ints = 0;
}

/**
 * .defchr pseudo instruction
 */
void pseudo_defchr() {
    int i = 0, j = 0, k = 0, l = 0, loops = 2, digit = 0;
    unsigned int byte = 0;

    if (length_ints != 8) {
        yyerror("Too few arguments. %d provided, need 8", length_ints);
    }

    while (loops != 0) {
        for (i = 0, j = 8; i < j; i++) {
            byte = 0;

            for (k = 0, l = 8; k < l; k++) {
                digit = (ints[i] / (int)pow(10, l - k - 1)) % 10;
                digit = (digit >> (loops - 1)) & 0x01;

                byte |= (digit << (l - k - 1));
            }

            write_byte(byte);
        }

        loops -= 1;
    }

    length_ints = 0;
}

/**
 * .dw pseudo instruction
 */
void pseudo_dw() {
    int i = 0, l = 0;

    for (i = 0, l = length_ints; i < l; i++) {
        write_byte(ints[i] & 0xFF);
        write_byte((ints[i] >> 8) & 0xFF);
    }

    length_ints = 0;
}

/**
 * .hibytes pseudo instruction
 */
void pseudo_hibytes() {
    int i = 0, l = 0;

    for (i = 0, l = length_ints; i < l; i++) {
        write_byte((ints[i] >> 8) & 0xFF);
    }

    length_ints = 0;
}

/**
 * .ineschr pseudo instruction
 * @param {int} value - Number of 8KB CHR banks
 */
void pseudo_ineschr(int value) {
    flags |= FLAG_NES;

    ines.chr = value;
}

/**
 * .inesmap pseudo instruction
 * @param {int} value - Mapper ID
 */
void pseudo_inesmap(int value) {
    flags |= FLAG_NES;

    ines.map = value;
}

/**
 * .inesmir pseudo instruction
 * @param {int} value - Mirroring value
 */
void pseudo_inesmir(int value) {
    flags |= FLAG_NES;

    ines.mir = value;
}

/**
 * .inesprg pseudo instruction
 * @param {int} value - Number of 16KB PRG banks
 */
void pseudo_inesprg(int value) {
    flags |= FLAG_NES;

    ines.prg = value;
}

/**
 * .inestrn pseudo instruction
 * @param {char *} string - Filename of trainer
 */
void pseudo_inestrn(char *string) {
    size_t path_length = 0, string_length = 0;
    char *path = NULL;

    string_length = strlen(string);
    path_length = strlen(cwd_path) + string_length - 1;
    path = (char *)malloc(sizeof(char) * (path_length + 1));

    if (!path) {
        yyerror("Memory error");
        goto cleanup;
    }

    strcpy(path, cwd_path);
    strcat(path, "/");
    strncat(path, string + 1, string_length - 2);

    flags |= FLAG_NES;

    ines.trn = 1;

    include_file_push(path);

cleanup:
    if (path) {
        free(path);
    }
}

/**
 * .incbin pseudo instruction
 * @param {char *} string - Filename of binary
 */
void pseudo_incbin(char *string, int offset, int limit) {
    int i = 0, l = 0;
    size_t path_length = 0, string_length = 0, bin_length = 0;
    char *path = NULL, *bin_data = NULL;
    FILE *incbin = NULL;

    string_length = strlen(string);
    path_length = strlen(cwd_path) + string_length - 1;
    path = (char *)malloc(sizeof(char) * (path_length + 1));

    if (!path) {
        yyerror("Memory error");
        goto cleanup;
    }

    strcpy(path, cwd_path);
    strcat(path, "/");
    strncat(path, string + 1, string_length - 2);

    incbin = fopen(path, "r");

    if (!incbin) {
        yyerror("Could not include %s", string);
        goto cleanup;
    }

    (void)fseek(incbin, 0, SEEK_END);
    bin_length = ftell(incbin);
    (void)fseek(incbin, 0, SEEK_SET);

    if (!bin_length) {
        goto cleanup;
    }

    bin_data = (char *)malloc(sizeof(char) * (bin_length + 1));

    if (!bin_data) {
        yyerror("Memory error");
        goto cleanup;
    }

    fread(bin_data, bin_length, 1, incbin);

    if (limit == -1) {
        limit = (int)bin_length;
    }

    for (i = offset, l = limit; i < l; i++) {
        write_byte((unsigned int)bin_data[i]);
    }

cleanup:
    if (path) {
        free(path);
    }
}

/**
 * .include pseudo instruction
 * @param {char *} string - Filename to include
 */
void pseudo_include(char *string) {
    size_t path_length = 0, string_length = 0;
    char *path = NULL;

    string_length = strlen(string);
    path_length = strlen(cwd_path) + string_length - 1;
    path = (char *)malloc(sizeof(char) * (path_length + 1));

    if (!path) {
        yyerror("Memory error");
        goto cleanup;
    }

    strcpy(path, cwd_path);
    strcat(path, "/");
    strncat(path, string + 1, string_length - 2);

    include_file_push(path);

cleanup:
    if (path) {
        free(path);
    }
}

/**
 * .lobytes pseudo instruction
 */
void pseudo_lobytes() {
    int i = 0, l = 0;

    for (i = 0, l = length_ints; i < l; i++) {
        write_byte(ints[i] & 0xFF);
    }

    length_ints = 0;
}

/**
 * .org pseudo instruction
 * @param {int} address - Organization address
 */
void pseudo_org(int address) {
    // TODO: add check for too high an address

    bank_offsets[bank_index] = address;
}

/**
 * .out pseudo instruction
 * @param {char *} string - Text to output
 */
void pseudo_out(char *string) {
    size_t length = 0;

    if (pass == 2) {
        length = strlen(string);
        string[length - 1] = '\0';

        fprintf(stderr, "%s\n", string+1);
    }
}

/**
 * .prg[0-9]+ pseudo instruction
 * @param {int} index - PRG bank index
 */
void pseudo_prg(int index) {
    segment_type = SEGMENT_PRG;

    prg_index = index;
}

/**
 * .rsset pseudo instruction
 * @param {int} address - Variable start address
 */
void pseudo_rsset(int address) {
    rsset = address;
}

/**
 * .rs pseudo instruction
 * @param {char *} label - Variable label
 * @param {int} size - Variable size
 */
void pseudo_rs(char *label, int size) {
    add_symbol(label, rsset, SYMBOL_RS);

    rsset += size;
}

/**
 * .segment pseudo instruction
 * @param {char *} string - Segment name
 */
void pseudo_segment(char *string) {
    size_t length = 0;

    length = strlen(string);

    strcpy(segment, string+1);
    segment[length-2] = '\0';

    if (length-2 > 3 && strncmp(segment, "PRG", 3) == 0) {
        pseudo_prg((int)strtol(segment+3, NULL, 10));
    }

    if (length-2 > 3 && strncmp(segment, "CHR", 3) == 0) {
        pseudo_chr((int)strtol(segment+3, NULL, 10));
    }
}

/**
 * Get opcode
 * @param {char *} mnemonic - Mnemonic
 * @param {int} mode - Addressing mode
 */
int get_opcode(char *mnemonic, int mode) {
    int i = 0, l = 0, opcode_index = -1;

    for (i = 0, l = OPCODE_COUNT; i < l; i++) {
        if (opcode_index == -1 && opcodes[i].mode == mode && strcmp(opcodes[i].mnemonic, mnemonic) == 0) {
            if (is_flag_undocumented()) {
                if ((opcodes[i].meta & META_UNDOCUMENTED) != 0) {
                    opcode_index = i;
                }
            } else {
                if ((opcodes[i].meta & META_UNDOCUMENTED) == 0) {
                    opcode_index = i;
                }
            }
        }
    }

    return opcode_index;
}

/**
 * Assemble absolute addressing mode
 * @param {char *} mnemonic - Mnemonic
 * @param {int} address - Address
 */
void assemble_absolute(char *mnemonic, int address) {
    unsigned int opcode_index = get_opcode(mnemonic, MODE_ABSOLUTE);

    if (opcode_index == -1) {
        opcode_index = get_opcode(mnemonic, MODE_RELATIVE);

        if (opcode_index != -1) {
            assemble_relative(mnemonic, address);
            return;
        }
    }

    if (opcode_index == -1) {
        yyerror("Unknown opcode `%s`", mnemonic);
    }

    address -= bank_index * BANK_SIZE;

    write_byte(opcode_index);
    write_byte(address & 0xFF);
    write_byte((address >> 8) & 0xFF);
}

/**
 * Assemble absolute_x/absolute_y addressing mode
 * @param {char *} mnemonic - Mnemonic
 * @param {int} address - Address
 * @param {char} reg - Register
 */
void assemble_absolute_xy(char *mnemonic, int address, char reg) {
    unsigned int opcode_index = -1;

    if (reg == 'X') {
        opcode_index = get_opcode(mnemonic, MODE_ABSOLUTE_X);
    } else if (reg == 'Y') {
        opcode_index = get_opcode(mnemonic, MODE_ABSOLUTE_Y);
    } else {
        yyerror("Unknown register `%c`", reg);
    }

    if (opcode_index == -1) {
        yyerror("Unknown opcode `%s`", mnemonic);
    }

    address -= bank_index * BANK_SIZE;

    write_byte(opcode_index);
    write_byte(address & 0xFF);
    write_byte((address >> 8) & 0xFF);
}

/**
 * Assemble accumulator addressing mode
 * @param {char *} mnemonic - Mnemonic
 */
void assemble_accumulator(char *mnemonic) {
    unsigned int opcode_index = get_opcode(mnemonic, MODE_ACCUMULATOR);

    if (opcode_index == -1) {
        yyerror("Unknown opcode `%s`", mnemonic);
    }

    write_byte(opcode_index);
}

/**
 * Assemble implied addressing mode
 * @param {char *} mnemonic - Mnemonic
 */
void assemble_implied(char *mnemonic) {
    unsigned int opcode_index = get_opcode(mnemonic, MODE_IMPLIED);

    if (opcode_index == -1) {
        yyerror("Unknown opcode `%s`", mnemonic);
    }

    write_byte(opcode_index);
}

/**
 * Assemble immediate addressing mode
 * @param {char *} mnemonic - Mnemonic
 * @param {int} value - Value
 */
void assemble_immediate(char *mnemonic, int value) {
    unsigned int opcode_index = get_opcode(mnemonic, MODE_IMMEDIATE);

    if (opcode_index == -1) {
        yyerror("Unknown opcode `%s`", mnemonic);
    }

    write_byte(opcode_index);
    write_byte(value & 0xFF);
}

/**
 * Assemble indirect addressing mode
 * @param {char *} mnemonic - Mnemonic
 * @param {int} address - Address
 */
void assemble_indirect(char *mnemonic, int address) {
    unsigned int opcode_index = get_opcode(mnemonic, MODE_INDIRECT);

    if (opcode_index == -1) {
        yyerror("Unknown opcode `%s`", mnemonic);
    }

    address -= bank_index * BANK_SIZE;

    write_byte(opcode_index);
    write_byte(address & 0xFF);
    write_byte((address >> 8) & 0xFF);
}

/**
 * Assemble indirect_x/indirect_y addressing mode
 * @param {char *} mnemonic - Mnemonic
 * @param {int} address - Address
 * @param {char} reg - Register
 */
void assemble_indirect_xy(char *mnemonic, int address, char reg) {
    unsigned int opcode_index = -1;

    if (reg == 'X') {
        opcode_index = get_opcode(mnemonic, MODE_INDIRECT_X);
    } else if (reg == 'Y') {
        opcode_index = get_opcode(mnemonic, MODE_INDIRECT_Y);
    } else {
        yyerror("Unknown register `%c`", reg);
    }

    if (opcode_index == -1) {
        yyerror("Unknown opcode `%s`", mnemonic);
    }

    address -= bank_index * BANK_SIZE;

    write_byte(opcode_index);
    write_byte(address & 0xFF);
}

/**
 * Assemble relative addressing mode
 * @param {char *} mnemonic - Mnemonic
 * @param {int} address - Address
 */
void assemble_relative(char *mnemonic, int address) {
    unsigned int opcode_id = get_opcode(mnemonic, MODE_RELATIVE);
    int offset = get_address_offset();

    address -= bank_index * BANK_PRG;

    if (offset > address) {
        address = 0xFE - (offset - address);
    } else {
        address = offset - address;
    }

    // TODO: throw error if jump is too large
    address &= 0xFF;

    write_byte(opcode_id);
    write_byte(address);
}

/**
 * Assemble zeropage addressing mode
 * @param {char *} mnemonic - Mnemonic
 * @param {int} address - Address
 */
void assemble_zeropage(char *mnemonic, int address) {
    unsigned int opcode_id = get_opcode(mnemonic, MODE_ZEROPAGE);

    address -= bank_index * BANK_SIZE;

    write_byte(opcode_id);
    write_byte(address & 0XFF);
}

/**
 * Assemble zeropage_x/zeropage_y addressing mode
 * @param {char *} mnemonic - Mnemonic
 * @param {int} address - Address
 * @param {char} reg - Register
 */
void assemble_zeropage_xy(char *mnemonic, int address, char reg) {
    unsigned int opcode_index = -1;

    if (reg == 'X') {
        opcode_index = get_opcode(mnemonic, MODE_ZEROPAGE_X);
    } else if (reg == 'Y') {
        opcode_index = get_opcode(mnemonic, MODE_ZEROPAGE_Y);
    } else {
        yyerror("Unknown register `%c`", reg);
    }

    if (opcode_index == -1) {
        yyerror("Unknown opcode `%s`", mnemonic);
    }

    address -= bank_index * BANK_SIZE;

    write_byte(opcode_index);
    write_byte(address & 0xFF);
}
