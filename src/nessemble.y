%{

#include <math.h>
#include "nessemble.h"

#define YYBAIL { include_file_original(); YYABORT; }

%}

%union {
    char *sval;
    char cval;
    int ival;
    unsigned int uval;
}

%token ENDL
%token INDENTATION

%token POW
%token MULT
%token DIV
%token AND
%token OR
%token XOR
%token RSHIFT
%token LSHIFT
%token MOD
%token DBL_EQU
%token NOT_EQU

%token HASH
%token COMMA
%token EQU
%token OPEN_PAREN
%token CLOSE_PAREN
%token OPEN_BRACK
%token CLOSE_BRACK
%token LT
%token GT
%token LTE
%token GTE
%token ARROW

%token HIGH
%token LOW
%token BANK

%token PSEUDO_ASCII
%token PSEUDO_BYTE
%token PSEUDO_CHECKSUM
%token PSEUDO_CHR
%token PSEUDO_COLOR
%token PSEUDO_DB
%token PSEUDO_DEFCHR
%token PSEUDO_DW
%token PSEUDO_ELSE
%token PSEUDO_ENDENUM
%token PSEUDO_ENDIF
%token PSEUDO_ENUM
%token PSEUDO_FILL
%token PSEUDO_FONT
%token PSEUDO_HIBYTES
%token PSEUDO_IF
%token PSEUDO_IFDEF
%token PSEUDO_IFNDEF
%token PSEUDO_INCBIN
%token PSEUDO_INCLUDE
%token PSEUDO_INCPAL
%token PSEUDO_INCPNG
%token PSEUDO_INCRLE
%token PSEUDO_INCWAV
%token PSEUDO_INESCHR
%token PSEUDO_INESMAP
%token PSEUDO_INESMIR
%token PSEUDO_INESPRG
%token PSEUDO_INESTRN
%token PSEUDO_LOBYTES
%token PSEUDO_MACRO
%token PSEUDO_MACRO_END
%token PSEUDO_ORG
%token PSEUDO_OUT
%token PSEUDO_PRG
%token PSEUDO_RANDOM
%token PSEUDO_RSSET
%token PSEUDO_RS
%token PSEUDO_SEGMENT
%token PSEUDO_WORD

%token <ival> PLUSSES
%token <ival> PLUS
%token <ival> MINUSES
%token <ival> MINUS
%token <ival> NUMBER_DEFCHR
%token <ival> NUMBER_HEX
%token <ival> NUMBER_BIN
%token <ival> NUMBER_OCT
%token <ival> NUMBER_DEC
%token <ival> NUMBER_CHAR
%token <ival> NUMBER_ARG
%token <ival> NUMBER_ARGC
%token <ival> NUMBER_ARGU

%token <sval> TEXT
%token <sval> COLON
%token <sval> QUOT_STRING
%token <sval> PSEUDO_MACRO_DEF
%token <sval> PSEUDO_MACRO_APPEND
%token <sval> PSEUDO_CUSTOM

%token <cval> CHAR_REG

%token FEOF
%token UNKNOWN

%type <sval> text
%type <sval> pseudo_ifdef
%type <sval> pseudo_ifndef
%type <sval> pseudo_incwav
%type <sval> pseudo_inestrn
%type <sval> pseudo_macro
%type <sval> pseudo_out
%type <sval> pseudo_segment
%type <sval> pseudo_custom

%type <ival> number
%type <ival> number_bank
%type <ival> number_base
%type <ival> number_defchr
%type <ival> number_highlow
%type <ival> label_text
%type <ival> local_label_text
%type <ival> label

%type <uval> pseudo_checksum
%type <uval> pseudo_chr
%type <uval> pseudo_enum
%type <uval> pseudo_if
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

endl
    : ENDL { /* NOTHING */ }
    | FEOF { /* NOTHING */ }
    ;

number
    : number_base                   { $$ = $1; }
    | label                         { $$ = $1; }
    | number POW number             { $$ = pow($1, $3); }
    | number MULT number            { $$ = $1 * $3; }
    | number DIV number             { $$ = (int)($1 / $3); }
    | number PLUS number            { $$ = $1 + $3; }
    | number MINUS number           { $$ = $1 - $3; }
    | number AND number             { $$ = $1 & $3; }
    | number OR number              { $$ = $1 | $3; }
    | number XOR number             { $$ = $1 ^ $3; }
    | number RSHIFT number          { $$ = $1 >> $3; }
    | number LSHIFT number          { $$ = $1 << $3; }
    | number MOD number             { $$ = $1 % $3; }
    | number DBL_EQU number         { $$ = $1 == $3 ? TRUE : FALSE; }
    | number NOT_EQU number         { $$ = $1 != $3 ? TRUE : FALSE; }
    | number LT number              { $$ = $1 < $3 ? TRUE : FALSE; }
    | number GT number              { $$ = $1 > $3 ? TRUE : FALSE; }
    | number LTE number             { $$ = $1 <= $3 ? TRUE : FALSE; }
    | number GTE number             { $$ = $1 >= $3 ? TRUE : FALSE; }
    | OPEN_PAREN number CLOSE_PAREN { $$ = $2; }
    ;

number_base
    : NUMBER_HEX     { $$ = $1; }
    | NUMBER_BIN     { $$ = $1; }
    | NUMBER_OCT     { $$ = $1; }
    | NUMBER_DEC     { $$ = $1; }
    | NUMBER_CHAR    { $$ = $1; }
    | NUMBER_ARG     { $$ = args[$1 - 1]; }
    | NUMBER_ARGC    { $$ = length_args; }
    | NUMBER_ARGU    { $$ = arg_unique; }
    | number_bank    { $$ = $1; }
    | number_highlow { $$ = $1; }
    ;

number_defchr
    : NUMBER_DEFCHR { $$ = $1; }
    | NUMBER_DEC    { $$ = $1; }
    ;

number_bank
    : BANK OPEN_PAREN text CLOSE_PAREN { if (pass == 2) { $$ = symbols[get_symbol($3)].bank; } else { if (get_symbol($3) != -1) { $$ = symbols[get_symbol($3)].bank; } else { $$ = 1; } } if (error_exists() == TRUE) { YYBAIL; } }
    ;

number_highlow
    : LOW OPEN_PAREN number CLOSE_PAREN  { $$ = $3 & 255; }
    | HIGH OPEN_PAREN number CLOSE_PAREN { $$ = ($3 >> 8) & 255; }
    ;

text
    : TEXT             { $$ = $1; }
    | TEXT NUMBER_ARGU { $$ = argu_macro($1); }
    ;

label_text
    : text { if (pass == 2) { $$ = symbols[get_symbol($1)].value; } else { if (get_symbol($1) != -1) { $$ = symbols[get_symbol($1)].value; } else { add_symbol($1, 1, SYMBOL_UNDEFINED); $$ = 1; } } if (error_exists() == TRUE) { YYBAIL; } }
    ;

local_label_text
    : COLON PLUSSES { if (pass == 2) { $$ = symbols[get_symbol_local($2)].value; } else { $$ = 1; } }
    | COLON MINUSES { if (pass == 2) { $$ = symbols[get_symbol_local($2)].value; } else { $$ = 1; } }
    | COLON PLUS    { if (pass == 2) { $$ = symbols[get_symbol_local($2)].value; } else { $$ = 1; } }
    | COLON MINUS   { if (pass == 2) { $$ = symbols[get_symbol_local($2)].value; } else { $$ = 1; } }
    ;

label
    : label_text             { $$ = $1; }
    | local_label_text       { $$ = $1; }
    | label ARROW label_text { $$ = $1 + $3; }
    ;

comma
    : COMMA             { /* NOTHING */ }
    | comma endl        { /* NOTHING */ }
    | comma INDENTATION { /* NOTHING */ }
    ;

/* Lines */

line
    : ENDL                    { /* NOTHING */ }
    | FEOF                    { if (include_type == INCLUDE_STRING) { include_string_pop(); } else { include_file_pop(); if (include_stack_ptr < 0) { YYACCEPT; } } }
    | INDENTATION             { /* NOTHING */ }
    | pseudo                  { /* NOTHING */ }
    | constant_decl           { /* NOTHING */ }
    | label_decl              { /* NOTHING */ }
    | INDENTATION instruction { /* NOTHING */ }
    ;

/* Pseudo */

pseudo
    : pseudo_ascii     { /* NOTHING */ }
    | pseudo_checksum  { pseudo_checksum($1); }
    | pseudo_chr       { pseudo_chr($1); }
    | pseudo_color     { pseudo_color(); }
    | pseudo_db        { pseudo_db(); }
    | pseudo_defchr    { pseudo_defchr(); }
    | pseudo_dw        { pseudo_dw(); }
    | pseudo_else      { pseudo_else(); }
    | pseudo_endenum   { pseudo_endenum(); }
    | pseudo_endif     { pseudo_endif(); }
    | pseudo_enum      { /* NOTHING */ }
    | pseudo_fill      { pseudo_fill(); }
    | pseudo_font      { pseudo_font(); }
    | pseudo_hibytes   { pseudo_hibytes(); }
    | pseudo_if        { pseudo_if($1); }
    | pseudo_ifdef     { pseudo_ifdef($1); }
    | pseudo_ifndef    { pseudo_ifndef($1); }
    | pseudo_incbin    { /* NOTHING */ }
    | pseudo_include   { if (error_exists() == TRUE) { YYBAIL; } }
    | pseudo_incpal    { if (error_exists() == TRUE) { YYBAIL; } }
    | pseudo_incpng    { if (error_exists() == TRUE) { YYBAIL; } }
    | pseudo_incrle    { /* NOTHING */ }
    | pseudo_incwav    { pseudo_incwav($1); if (error_exists() == TRUE) { YYBAIL; } }
    | pseudo_ineschr   { pseudo_ineschr($1); }
    | pseudo_inesmap   { pseudo_inesmap($1); }
    | pseudo_inesmir   { pseudo_inesmir($1); }
    | pseudo_inesprg   { pseudo_inesprg($1); }
    | pseudo_inestrn   { pseudo_inestrn($1); }
    | pseudo_lobytes   { pseudo_lobytes(); }
    | pseudo_macro     { pseudo_macro($1); if (error_exists() == TRUE) { YYBAIL; } }
    | pseudo_macro_def { /* NOTHING */ }
    | pseudo_org       { pseudo_org($1); }
    | pseudo_out       { pseudo_out($1); }
    | pseudo_prg       { pseudo_prg($1); }
    | pseudo_random    { pseudo_random(); }
    | pseudo_rsset     { pseudo_rsset($1); }
    | pseudo_rs        { /* NOTHING */ }
    | pseudo_segment   { pseudo_segment($1); }
    | pseudo_custom    { pseudo_custom($1); }
    ;

pseudo_ascii
    : PSEUDO_ASCII QUOT_STRING              { pseudo_ascii($2, 0); }
    | PSEUDO_ASCII QUOT_STRING PLUS number  { pseudo_ascii($2, $4); }
    | PSEUDO_ASCII QUOT_STRING MINUS number { pseudo_ascii($2, $4 * -1); }
    ;

pseudo_checksum
    : PSEUDO_CHECKSUM label { $$ = $2; }
    ;

pseudo_chr
    : PSEUDO_CHR number { $$ = $2; }
    ;

pseudo_color
    : PSEUDO_COLOR number       { ints[length_ints++] = $2; }
    | pseudo_color COMMA number { ints[length_ints++] = $3; }
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
    : PSEUDO_DEFCHR number_defchr       { ints[length_ints++] = $2; }
    | pseudo_defchr comma number_defchr { ints[length_ints++] = $3; }
    ;

pseudo_dw_alias
    : PSEUDO_DW   { /* NOTHING */ }
    | PSEUDO_WORD { /* NOTHING */ }
    ;

pseudo_dw
    : pseudo_dw_alias number { ints[length_ints++] = $2; }
    | pseudo_dw COMMA number { ints[length_ints++] = $3; }
    ;

pseudo_else
    : PSEUDO_ELSE
    ;

pseudo_endenum
    : PSEUDO_ENDENUM
    ;

pseudo_endif
    : PSEUDO_ENDIF
    ;

pseudo_enum
    : PSEUDO_ENUM number       { pseudo_enum($2, 1); $$ = $2; }
    | pseudo_enum COMMA number { pseudo_enum($1, $3); }
    ;

pseudo_fill
    : PSEUDO_FILL number       { ints[length_ints++] = $2; }
    | pseudo_fill COMMA number { ints[length_ints++] = $3; }
    ;

pseudo_font
    : PSEUDO_FONT number       { ints[length_ints++] = $2; }
    | pseudo_font COMMA number { ints[length_ints++] = $3; }
    ;

pseudo_hibytes
    : PSEUDO_HIBYTES number       { ints[length_ints++] = $2; }
    | pseudo_hibytes COMMA number { ints[length_ints++] = $3; }
    ;

pseudo_if
    : PSEUDO_IF number { $$ = $2; }
    ;

pseudo_ifdef
    : PSEUDO_IFDEF text { $$ = $2; }
    ;

pseudo_ifndef
    : PSEUDO_IFNDEF text { $$ = $2; }
    ;

pseudo_incbin
    : PSEUDO_INCBIN QUOT_STRING                           { pseudo_incbin($2, 0, -1); }
    | PSEUDO_INCBIN QUOT_STRING COMMA number              { pseudo_incbin($2, $4, -1); }
    | PSEUDO_INCBIN QUOT_STRING COMMA number COMMA number { pseudo_incbin($2, $4, $6); }
    ;

pseudo_include
    : PSEUDO_INCLUDE QUOT_STRING endl { pseudo_include($2); }
    ;

pseudo_incpal
    : PSEUDO_INCPAL QUOT_STRING { pseudo_incpal($2); }
    ;

pseudo_incpng
    : PSEUDO_INCPNG QUOT_STRING                           { pseudo_incpng($2, 0, -1); }
    | PSEUDO_INCPNG QUOT_STRING COMMA number              { pseudo_incpng($2, $4, -1); }
    | PSEUDO_INCPNG QUOT_STRING COMMA number COMMA number { pseudo_incpng($2, $4, $6); }
    ;

pseudo_incwav
    : PSEUDO_INCWAV QUOT_STRING  { $$ = $2; }
    | pseudo_incwav COMMA number { ints[length_ints++] = $3; $$ = $1; }
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

pseudo_incrle
    : PSEUDO_INCRLE QUOT_STRING { pseudo_incrle($2); }
    ;

pseudo_inestrn
    : PSEUDO_INESTRN QUOT_STRING { $$ = $2; }
    ;

pseudo_lobytes
    : PSEUDO_LOBYTES number       { ints[length_ints++] = $2; }
    | pseudo_lobytes COMMA number { ints[length_ints++] = $3; }
    ;

pseudo_macro
    : PSEUDO_MACRO text         { length_args = 0; $$ = $2; }
    | pseudo_macro COMMA number { args[length_args++] = $3; $$ = $1; }
    ;

pseudo_macro_def
    : PSEUDO_MACRO_DEF    { add_macro($1); }
    | PSEUDO_MACRO_END    { end_macro(); }
    | PSEUDO_MACRO_APPEND { append_macro($1); }
    ;

pseudo_org
    : PSEUDO_ORG number { $$ = $2; }
    ;

pseudo_out
    : PSEUDO_OUT QUOT_STRING { $$ = $2; }
    ;

pseudo_prg
    : PSEUDO_PRG number { $$ = $2; }
    ;

pseudo_random
    : PSEUDO_RANDOM QUOT_STRING  { ints[length_ints++] = str2hash($2); }
    | PSEUDO_RANDOM number       { ints[length_ints++] = $2; }
    | pseudo_random COMMA number { ints[length_ints++] = $3; }
    | PSEUDO_RANDOM              { /* NOTHING */ }
    ;

pseudo_rsset
    : PSEUDO_RSSET number { $$ = $2; }
    ;

pseudo_rs
    : text PSEUDO_RS number { pseudo_rs($1, $3); }
    ;

pseudo_segment
    : PSEUDO_SEGMENT QUOT_STRING { $$ = $2; }
    ;

pseudo_custom
    : PSEUDO_CUSTOM                   { /* NOTHING */ }
    | PSEUDO_CUSTOM number            { ints[length_ints++] = $2; $$ = $1; }
    | PSEUDO_CUSTOM QUOT_STRING       { texts[length_texts++] = nessemble_strdup($2); $$ = $1; }
    | pseudo_custom COMMA number      { ints[length_ints++] = $3; $$ = $1; }
    | pseudo_custom COMMA QUOT_STRING { texts[length_texts++] = nessemble_strdup($3); $$ = $1; }
    ;

/* Constant */

constant_decl
    : text            { add_constant($1, 0x01); }
    | text EQU number { add_constant($1, $3); }
    ;

/* Label */

label_decl
    : text COLON { add_label($1); }
    | COLON      { add_label($1); }
    ;

/* Instruction */

instruction
    : TEXT CHAR_REG                                     { assemble_accumulator($1, $2); }
    | TEXT HASH number                                  { assemble_immediate($1, $3); }
    | TEXT OPEN_BRACK number CLOSE_BRACK                { assemble_indirect($1, $3); }
    | TEXT OPEN_BRACK number COMMA CHAR_REG CLOSE_BRACK { assemble_indirect_xy($1, $3, $5); }
    | TEXT OPEN_BRACK number CLOSE_BRACK COMMA CHAR_REG { assemble_indirect_xy($1, $3, $6); }
    | TEXT LT number COMMA CHAR_REG                     { assemble_zeropage_xy($1, $3, $5); }
    | TEXT number COMMA CHAR_REG                        { assemble_absolute_xy($1, $2, $4); }
    | TEXT LT number endl                               { assemble_zeropage($1, $3); }
    | TEXT number endl                                  { assemble_absolute($1, $2); }
    | TEXT endl                                         { assemble_implied($1); }
    ;

%%
