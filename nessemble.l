%{

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "y.tab.h"
#include "nessemble.h"

// include
YY_BUFFER_STATE include_stack[MAX_INCLUDE_DEPTH];
int include_stack_ptr = 0;

%}

%option noinput
%option nounput
%option yylineno

%x include

%%

\n                     { return ENDL; }
^[ \t]+                { return INDENTATION; }

[ ]+                   { /* WHITESPACE */ }
;.*                    { /* COMMENT */ }

"+"                    { return PLUS; }
"-"                    { return MINUS; }
"*"                    { return MULT; }
"/"                    { return DIV; }
"&"                    { return AND; }
"|"                    { return OR; }
"^"                    { return XOR; }
">>"                   { return RSHIFT; }
"<<"                   { return LSHIFT; }

"#"                    { return HASH; }
","                    { return COMMA; }
"="                    { return EQU; }
":"                    { return COLON; }
"("                    { return OPEN_PAREN; }
")"                    { return CLOSE_PAREN; }
"["                    { return OPEN_BRACK; }
"]"                    { return CLOSE_BRACK; }
"<"                    { return CARET; }

"HIGH"                 { return HIGH; }
"LOW"                  { return LOW; }

".ascii"               { return PSEUDO_ASCII; }
".byte"                { return PSEUDO_BYTE; }
\.chr[0-9]+            { yylval.uval = (unsigned int)dec2int(yytext+4); return PSEUDO_CHR; }
".db"                  { return PSEUDO_DB; }
".defchr"              { return PSEUDO_DEFCHR; }
".dw"                  { return PSEUDO_DW; }
".else"                { return PSEUDO_ELSE; }
".endif"               { return PSEUDO_ENDIF; }
".hibytes"             { return PSEUDO_HIBYTES; }
".ifdef"               { return PSEUDO_IFDEF; }
".ifndef"              { return PSEUDO_IFNDEF; }
".incbin"              { return PSEUDO_INCBIN; }
".include"             { BEGIN(include); return PSEUDO_INCLUDE; }
".incpng"              { return PSEUDO_INCPNG; }
".ineschr"             { return PSEUDO_INESCHR; }
".inesmap"             { return PSEUDO_INESMAP; }
".inesmir"             { return PSEUDO_INESMIR; }
".inesprg"             { return PSEUDO_INESPRG; }
".inestrn"             { return PSEUDO_INESTRN; }
".lobytes"             { return PSEUDO_LOBYTES; }
".org"                 { return PSEUDO_ORG; }
".out"                 { return PSEUDO_OUT; }
\.prg[0-9]+            { yylval.uval = (unsigned int)dec2int(yytext+4); return PSEUDO_PRG; }
".rsset"               { return PSEUDO_RSSET; }
".rs"                  { return PSEUDO_RS; }
".segment"             { return PSEUDO_SEGMENT; }
".word"                { return PSEUDO_WORD; }

<include>[ \t]*        { /* WHITESPACE */ }
<include>[^ \t\n]+     { BEGIN(INITIAL); yylval.sval = strdup(yytext); return QUOT_STRING; }

\$[0-9a-fA-F]+         { yylval.ival = hex2int(yytext+1); return NUMBER_HEX; }
%[0-1]+                { yylval.ival = bin2int(yytext+1); return NUMBER_BIN; }
0[0-7]+                { yylval.ival = oct2int(yytext+1); return NUMBER_OCT; }
[0-9]+                 { yylval.ival = dec2int(yytext); return NUMBER_DEC; }
'.'                    { yylval.ival = (int)yytext[1]; return NUMBER_CHAR; }

[a-zA-Z_][a-zA-Z0-9_]+ { yylval.sval = strdup(yytext); return TEXT; }
\".+\"                 { yylval.sval = strdup(yytext); return QUOT_STRING; }
[aA]                   { yylval.cval = toupper(yytext[0]); return CHAR_A; }
[xX]                   { yylval.cval = toupper(yytext[0]); return CHAR_X; }
[yY]                   { yylval.cval = toupper(yytext[0]); return CHAR_Y; }

<<EOF>>                { return FEOF; }
.                      { if (strdup(yytext)[0] == 13) { return ENDL; } else { return UNKNOWN; } }

%%

/**
 * Pushes an .include file to the buffer
 * @param {char} filename - Filename to include
 */
void include_file_push(char *filename) {
    // push to include stack
    include_stack[include_stack_ptr++] = YY_CURRENT_BUFFER;

    // open file
    yyin = fopen(filename, "r");

    // check if opened
    if (!yyin) {
        fprintf(stderr, "Could not include %s\n", filename);
        return;
    }

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
}
