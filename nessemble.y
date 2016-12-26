%{

#include "nessemble.h"

%}

%union {
    char *sval;
    char cval;
    int ival;
    unsigned int uval;
}

%token ENDL
%token INDENTATION

%token PLUS
%token MINUS
%token MULT
%token DIV
%token AND
%token OR
%token XOR

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

%type <uval> pseudo_chr
%type <uval> pseudo_ineschr
%type <uval> pseudo_inesmap
%type <uval> pseudo_inesmir
%type <uval> pseudo_inesprg
%type <uval> pseudo_org
%type <uval> pseudo_prg
%type <uval> pseudo_rsset

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
    | label                    { $$ = $1; }
    | number PLUS number_base  { $$ = $1 + $3; }
    | number MINUS number_base { $$ = $1 - $3; }
    | number MULT number_base  { $$ = $1 * $3; }
    | number DIV number_base   { $$ = (int)($1 / $3); }
    | number AND number_base   { $$ = $1 & $3; }
    | number OR number_base    { $$ = $1 | $3; }
    | number XOR number_base    { $$ = $1 ^ $3; }
    ;

number_base
    : NUMBER_HEX     { $$ = $1; }
    | NUMBER_BIN     { $$ = $1; }
    | NUMBER_OCT     { $$ = $1; }
    | NUMBER_DEC     { $$ = $1; }
    | number_highlow { $$ = $1; }
    ;

number_highlow
    : LOW open_char number close_char  { $$ = $3 & 255; }
    | HIGH open_char number close_char { $$ = ($3 >> 8) & 255; }
    ;

label
    : TEXT { if (pass == 2) { $$ = symbols[get_symbol($1)].value; } else { $$ = 0; } }
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

pseudo_chr
    : PSEUDO_CHR { $$ = $1; }
    ;

pseudo_db_alias
    : PSEUDO_DB   { /* NOTHING */ }
    | PSEUDO_BYTE { /* NOTHING */ }
    ;

pseudo_db
    : pseudo_db_alias number { ints[length_ints++] = $2; }
    | pseudo_db COMMA number { ints[length_ints++] = $3; }
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
    | pseudo_dw COMMA number { ints[length_ints++] = $3; }
    ;

pseudo_hibytes
    : PSEUDO_HIBYTES number       { ints[length_ints++] = $2; }
    | pseudo_hibytes COMMA number { ints[length_ints++] = $3; }
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
    ;

pseudo_inesmap
    : PSEUDO_INESMAP number { $$ = $2; }
    ;

pseudo_inesmir
    : PSEUDO_INESMIR number { $$ = $2; }
    ;

pseudo_inesprg
    : PSEUDO_INESPRG number { $$ = $2; }
    ;

pseudo_inestrn
    : PSEUDO_INESTRN QUOT_STRING { $$ = $2; }
    ;

pseudo_lobytes
    : PSEUDO_LOBYTES number       { ints[length_ints++] = $2; }
    | pseudo_lobytes COMMA number { ints[length_ints++] = $3; }
    ;

pseudo_org
    : PSEUDO_ORG number { $$ = $2; }
    ;

pseudo_out
    : PSEUDO_OUT QUOT_STRING { $$ = $2; }
    ;

pseudo_prg
    : PSEUDO_PRG { $$ = $1; }
    ;

pseudo_rsset
    : PSEUDO_RSSET number { $$ = $2; }
    ;

pseudo_rs
    : TEXT PSEUDO_RS number { pseudo_rs($1, $3); }
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
    | TEXT open_char number close_char              { assemble_indirect($1, $3); }
    | TEXT open_char number COMMA CHAR_X close_char { assemble_indirect_xy($1, $3, $5); }
    | TEXT open_char number close_char COMMA CHAR_Y { assemble_indirect_xy($1, $3, $6); }
    | TEXT CARET number COMMA CHAR_X                { assemble_zeropage_xy($1, $3, $5); }
    | TEXT CARET number COMMA CHAR_Y                { assemble_zeropage_xy($1, $3, $5); }
    | TEXT number COMMA CHAR_X                      { assemble_absolute_xy($1, $2, $4); }
    | TEXT number COMMA CHAR_Y                      { assemble_absolute_xy($1, $2, $4); }
    | TEXT CARET number ENDL                        { assemble_zeropage($1, $3); }
    | TEXT number ENDL                              { assemble_absolute($1, $2); }
    | TEXT ENDL                                     { assemble_implied($1); }
    ;

%%
