%{

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "y.tab.h"
#include "nessemble.h"

// include
YY_BUFFER_STATE include_stack[MAX_INCLUDE_DEPTH];
int line_number_stack[MAX_INCLUDE_DEPTH];
int include_stack_ptr = 0;

%}

%option noinput
%option nounput
%option yylineno

%x include
%x macro

%%

\n                         { return ENDL; }
^[ \t]+                    { return INDENTATION; }

[ ]+                       { /* WHITESPACE */ }
;.*                        { /* COMMENT */ }

"+"                        { return PLUS; }
"-"                        { return MINUS; }
"**"                       { return POW; }
"*"                        { return MULT; }
"/"                        { return DIV; }
"&"                        { return AND; }
"|"                        { return OR; }
"^"                        { return XOR; }
">>"                       { return RSHIFT; }
"<<"                       { return LSHIFT; }
"%"                        { return MOD; }
"=="                       { return DBL_EQU; }
"!="                       { return NOT_EQU; }

"#"                        { return HASH; }
","                        { return COMMA; }
"="                        { return EQU; }
":"                        { return COLON; }
"("                        { return OPEN_PAREN; }
")"                        { return CLOSE_PAREN; }
"["                        { return OPEN_BRACK; }
"]"                        { return CLOSE_BRACK; }
"<"                        { return LT; }
">"                        { return GT; }
"<="                       { return LTE; }
">="                       { return GTE; }

"HIGH"                     { return HIGH; }
"LOW"                      { return LOW; }

".ascii"                   { return PSEUDO_ASCII; }
".byte"                    { return PSEUDO_BYTE; }
".checksum"                { return PSEUDO_CHECKSUM; }
".chr"                     { return PSEUDO_CHR; }
".db"                      { return PSEUDO_DB; }
".defchr"                  { return PSEUDO_DEFCHR; }
".dw"                      { return PSEUDO_DW; }
".else"                    { return PSEUDO_ELSE; }
".endenum"                 { return PSEUDO_ENDENUM; }
".endif"                   { return PSEUDO_ENDIF; }
".enum"                    { return PSEUDO_ENUM; }
".fill"                    { return PSEUDO_FILL; }
".hibytes"                 { return PSEUDO_HIBYTES; }
".if"                      { return PSEUDO_IF; }
".ifdef"                   { return PSEUDO_IFDEF; }
".ifndef"                  { return PSEUDO_IFNDEF; }
".incbin"                  { return PSEUDO_INCBIN; }
".include"                 { BEGIN(include); return PSEUDO_INCLUDE; }
".incpal"                  { return PSEUDO_INCPAL; }
".incpng"                  { return PSEUDO_INCPNG; }
".incrle"                  { return PSEUDO_INCRLE; }
".incscreen"               { return PSEUDO_INCSCREEN; }
".incwav"                  { return PSEUDO_INCWAV; }
".ineschr"                 { return PSEUDO_INESCHR; }
".inesmap"                 { return PSEUDO_INESMAP; }
".inesmir"                 { return PSEUDO_INESMIR; }
".inesprg"                 { return PSEUDO_INESPRG; }
".inestrn"                 { return PSEUDO_INESTRN; }
".lobytes"                 { return PSEUDO_LOBYTES; }
".macro"                   { return PSEUDO_MACRO; }
\.macrodef.+               { BEGIN(macro); yylval.sval = strdup(yytext+10); return PSEUDO_MACRO_DEF; }
".org"                     { return PSEUDO_ORG; }
".out"                     { return PSEUDO_OUT; }
".prg"                     { return PSEUDO_PRG; }
".rsset"                   { return PSEUDO_RSSET; }
".rs"                      { return PSEUDO_RS; }
".segment"                 { return PSEUDO_SEGMENT; }
".word"                    { return PSEUDO_WORD; }

<include>[ \t]*            { /* WHITESPACE */ }
<include>[^ \t\n]+         { BEGIN(INITIAL); yylval.sval = strdup(yytext); return QUOT_STRING; }

<macro>\n                  { /* WHITESPACE */ }
<macro>\.endm              { BEGIN(INITIAL); return PSEUDO_MACRO_END; }
<macro>.+                  { yylval.sval = strdup(yytext); return PSEUDO_MACRO_APPEND; }

\$[0-9a-fA-F]+             { yylval.ival = hex2int(yytext+1); return NUMBER_HEX; }
[0-9a-fA-F]+h              { yylval.ival = hex2int(yytext); return NUMBER_HEX; }
%[0-1]+                    { yylval.ival = bin2int(yytext+1); return NUMBER_BIN; }
[0-1]+b                    { yylval.ival = bin2int(yytext); return NUMBER_BIN; }
0[0-7]+                    { yylval.ival = oct2int(yytext+1); return NUMBER_OCT; }
[0-7]+o                    { yylval.ival = oct2int(yytext); return NUMBER_OCT; }
[0-9]+                     { yylval.ival = dec2int(yytext); return NUMBER_DEC; }
[0-9]+d                    { yylval.ival = dec2int(yytext); return NUMBER_DEC; }
'.'                        { yylval.ival = (int)yytext[1]; return NUMBER_CHAR; }
\\[0-9]+                   { yylval.ival = dec2int(yytext+1); return NUMBER_ARG; }

[\.\@a-zA-Z_][a-zA-Z0-9_]+ { yylval.sval = strdup(yytext); return TEXT; }
\"[^"]*\"                  { yylval.sval = strdup(yytext); return QUOT_STRING; }
[a-zA-Z]                   { yylval.cval = toupper(yytext[0]); return CHAR_REG; }

<<EOF>>                    { return FEOF; }
.                          { if (strdup(yytext)[0] == 13) { return ENDL; } else { return UNKNOWN; } }

%%

/**
 * Pushes an .include file to the buffer
 * @param {char *} filename - Filename to include
 */
void include_file_push(char *filename) {
    size_t length = strlen(cwd_path) + 1;

    include_type = INCLUDE_FILE;

    // save line number
    line_number_stack[include_stack_ptr] = yylineno;

    // push to include stack
    include_stack[include_stack_ptr++] = YY_CURRENT_BUFFER;

    // error on too big include depth
    if (include_stack_ptr >= MAX_INCLUDE_DEPTH) {
        include_stack_ptr--;
        error_add("Too many nested includes");
        return;
    }

    // open file
    yyin = fopen(filename, "r");

    // check if opened
    if (!yyin) {
        include_stack_ptr--;
        error_add("Could not include `%s`", filename+length);
        return;
    }

    // reset line number
    yylineno = 0;

    // save filename
    strcpy(filename_stack[include_stack_ptr], filename);

    // switch to buffer
    yy_switch_to_buffer(yy_create_buffer(yyin, YY_BUF_SIZE));

    // continue with lexing
    BEGIN(INITIAL);
}

/**
 * Pops previous buffer after .include
 */
void include_file_pop() {
    // if all popped, end pass
    if (--include_stack_ptr < 0) {
        end_pass();
        return;
    }

    // delete buffer
    yy_delete_buffer(YY_CURRENT_BUFFER);

    // switch to next buffer
    yy_switch_to_buffer(include_stack[include_stack_ptr]);

    // restore line number
    yylineno = line_number_stack[include_stack_ptr];
}

/**
 * Pushes a string to the buffer
 * @param {char *} string - String to parse
 */
void include_string_push(char *string) {
    include_type = INCLUDE_STRING;

    // push to include stack
    include_stack[include_stack_ptr++] = YY_CURRENT_BUFFER;

    // switch to buffer
    yy_switch_to_buffer(yy_scan_string(string));

    // continue with lexing
    BEGIN(INITIAL);
}

/**
 * Pops previous buffer after string
 */
void include_string_pop() {
    include_type = INCLUDE_FILE;

    // delete buffer
    yy_delete_buffer(YY_CURRENT_BUFFER);

    // switch to next buffer
    yy_switch_to_buffer(include_stack[--include_stack_ptr]);
}
