#ifndef _NESSEMBLE_H
#define _NESSEMBLE_H

#include <stdio.h>

/*
 * DEFINES
 */

/* UNUSED */
#define UNUSED(x) (void)(x)

/* BOOLEAN */
#define TRUE  1
#define FALSE 0

/* RETURN CODES */
#define RETURN_OK    0
#define RETURN_EPERM 1
#define RETURN_USAGE 129

/* VARS */
#define MAX_INCLUDE_DEPTH 10
#define BANK_PRG          0x4000
#define BANK_CHR          0x2000
#define BANK_SIZE         0x4000
#define MAX_SYMBOLS       65536
#define MAX_MACROS        1024
#define MAX_BANKS         256
#define MAX_INTS          256
#define MAX_ARGS          10
#define MAX_NESTED_IFS    10
#define TRAINER_MAX       512

/* PATH */
#ifndef PATH_MAX
#define PATH_MAX          4096
#endif

/* CLI FLAGS */
#define FLAG_UNDOCUMENTED 0x01
#define FLAG_NES          0x02
#define FLAG_DISASSEMBLE  0x04
#define FLAG_SIMULATE     0x08
#define FLAG_CHECK        0x10

/* SEGMENTS */
#define SEGMENT_CHR 1
#define SEGMENT_PRG 2

/* MODES */
#define MODE_IMPLIED     0x00
#define MODE_ACCUMULATOR 0x01
#define MODE_RELATIVE    0x02
#define MODE_IMMEDIATE   0x03
#define MODE_ZEROPAGE    0x04
#define MODE_ZEROPAGE_X  0x05
#define MODE_ZEROPAGE_Y  0x06
#define MODE_ABSOLUTE    0x07
#define MODE_ABSOLUTE_Y  0x08
#define MODE_ABSOLUTE_X  0x09
#define MODE_INDIRECT    0x0A
#define MODE_INDIRECT_X  0x0B
#define MODE_INDIRECT_Y  0x0C

/* OPCODES */
#define OPCODE_COUNT      256
#define META_NONE         0x00
#define META_BOUNDARY     0x01
#define META_UNDOCUMENTED 0x02

/* SYMBOLS */
#define SYMBOL_UNDEFINED 0x00
#define SYMBOL_LABEL     0x01
#define SYMBOL_CONSTANT  0x02
#define SYMBOL_RS        0x03
#define SYMBOL_ENUM      0x04

/* INCLUDE */
#define INCLUDE_NONE   0
#define INCLUDE_FILE   1
#define INCLUDE_STRING 2

/* IF */
#define IF_IF     0
#define IF_IFDEF  1
#define IF_IFNDEF 2

/* SIMULATE */
#define REGISTER_A  1
#define REGISTER_X  2
#define REGISTER_Y  3
#define REGISTER_PC 4
#define REGISTER_SP 5
#define FLG_NEGATIVE  1
#define FLG_OVERFLOW  2
#define FLG_BREAK     3
#define FLG_DECIMAL   4
#define FLG_INTERRUPT 5
#define FLG_ZERO      6
#define FLG_CARRY     7

/* USAGE */
#define USAGE_FLAG_COUNT            9
#define SIMULATION_USAGE_FLAG_COUNT 16

/*
 * STRUCTS
 */

/* OPCODES */
struct opcode {
    char mnemonic[4];
    unsigned int mode, opcode, length, timing, meta;
    void (*func)(unsigned int, unsigned int);
};

/* SYMBOLS */
struct symbol {
    char *name;
    unsigned int value, type;
};

/* MACROS */
struct macro {
    char *name, *text;
};

/* INES */
struct ines_header {
    unsigned int chr, map, mir, prg, trn;
};

/* USAGE */
struct usage_flag {
    char *invocation, *description;
};

/*
 * VARIABLES
 */

/* FLAGS */
unsigned int flags;

/* OPCODES */
struct opcode opcodes[OPCODE_COUNT];

/* BISON */
int yylineno;
int yylex();
int yyparse();
FILE *yyin;

/* SYMBOLS */
struct symbol symbols[MAX_SYMBOLS];
unsigned int symbol_index;
unsigned int rsset;

/* ENUM */
unsigned int enum_active, enum_value, enum_inc;

/* MACROS */
struct macro macros[MAX_MACROS];
unsigned int macro_index;
unsigned int length_args;
unsigned int args[MAX_ARGS];

/* INCLUDE */
char filename_stack[MAX_INCLUDE_DEPTH][PATH_MAX];
unsigned int include_type;

/* IF */
unsigned int if_depth;
unsigned int if_active;
unsigned int if_cond[MAX_NESTED_IFS];

/* SEGMENTS */
char segment[8];
unsigned int segment_type;

/* CWD */
char cwd[PATH_MAX + 1];
char *cwd_path;

/* INPUT */
unsigned int length_ints;
unsigned int ints[MAX_INTS];

/* ERROR REPORTING */
char linebuf[512];
char yycolno;

/* INES */
extern struct ines_header ines;

/* IO */
unsigned int *rom;

/* OFFSETS */
unsigned int prg_offsets[MAX_BANKS];
unsigned int chr_offsets[MAX_BANKS];
unsigned int prg_index;
unsigned int chr_index;
int offset_max;

/* TRAINER */
unsigned int trainer[TRAINER_MAX];
unsigned int offset_trainer;

/* INCLUDE VARS */
int include_stack_ptr;

/* EOF VARS */
unsigned int pass;

/*
 * FUNCTIONS
 */

/* MAIN */
int main(int argc, char *argv[]);

/* USAGE */
int usage(char *exec);
void usage_simulate();

/* INCLUDE UTILS */
void include_file_pop();
void include_file_push(char *filename);
void include_file_original();
void include_string_push(char *string);
void include_string_pop();

/* FLAG TESTS */
unsigned int is_flag_undocumented();
unsigned int is_flag_nes();
unsigned int is_flag_disassemble();
unsigned int is_flag_simulate();
unsigned int is_flag_check();

/* SEGMENT TESTS */
unsigned int is_segment_chr();
unsigned int is_segment_prg();

/* EOF UTILS */
void assemble_feof();
void end_pass();

/* FILE LOCATION UTILS */
unsigned int get_rom_index();
unsigned int get_address_offset();
void write_byte(unsigned int byte);

/* SYMBOL UTILS */
void add_symbol(char *name, unsigned int value, unsigned int type);
int get_symbol(char *name);
void add_constant(char *name, unsigned int value);
void add_label(char *name);
int has_label(char *name);

/* PSEUDO UTILS */
void pseudo_ascii(char *string, int offset);
void pseudo_checksum(unsigned int address);
void pseudo_chr(unsigned int index);
void pseudo_db();
void pseudo_defchr();
void pseudo_dw();
void pseudo_else();
void pseudo_endenum();
void pseudo_endif();
void pseudo_enum(unsigned int value, unsigned int inc);
void pseudo_fill(unsigned int count, unsigned int value);
void pseudo_hibytes();
void pseudo_if(unsigned int cond);
void pseudo_ifdef(char *label);
void pseudo_ifndef(char *label);
void pseudo_ineschr(unsigned int value);
void pseudo_inesmap(unsigned int value);
void pseudo_inesmir(unsigned int value);
void pseudo_inesprg(unsigned int value);
void pseudo_inestrn(char *string);
void pseudo_incbin(char *string, int offset, int limit);
void pseudo_include(char *string);
void pseudo_incpal(char *string);
void pseudo_incpng(char *string, int offset, int limit);
void pseudo_incscreen(char *string, char *type);
void pseudo_incwav(char *string, int amplitude);
void pseudo_lobytes();
void pseudo_macro(char *string);
void pseudo_org(unsigned int address);
void pseudo_out(char *string);
void pseudo_prg(unsigned int index);
void pseudo_incrle(char *string);
void pseudo_rsset(unsigned int address);
void pseudo_rs(char *label, int size);
void pseudo_segment(char *string);

/* OPCODE UTILS */
int get_opcode(char *mnemonic, unsigned int mode);

/* MACRO UTILS */
void add_macro(char *name);
void end_macro();
void append_macro(char *text);
int get_macro(char *name);

/* ASSEMBLY UTILS */
void assemble_absolute(char *mnemonic, unsigned int address);
void assemble_absolute_xy(char *mnemonic, unsigned int address, char reg);
void assemble_accumulator(char *mnemonic, char reg);
void assemble_implied(char *mnemonic);
void assemble_immediate(char *mnemonic, unsigned int value);
void assemble_indirect(char *mnemonic, unsigned int address);
void assemble_indirect_xy(char *mnemonic, unsigned int address, char reg);
void assemble_relative(char *mnemonic, unsigned int address);
void assemble_zeropage(char *mnemonic, unsigned int address);
void assemble_zeropage_xy(char *mnemonic, unsigned int address, char reg);

/* DISASSEMBLY UTILS */
int disassemble(char *input, char *output);

/* SIMULATE UTILS */
int simulate(char *input, char *recipe);
void usage_simulate();
int repl(char *input);
void print_registers();
void load_registers(char *input);
void load_flags(char *input);
void fill_memory(char **output, char *input);
unsigned int print_instruction(unsigned int address);
void print_instructions(char *input);
int step();
int steps(char *input);
void print_memory(char *input);
void load_goto(char *input);
void print_cycles();
void start_record(char *input);
void list_breakpoints();
void add_breakpoint(char *input);
void remove_breakpoint(char *input);
unsigned int at_breakpoint();
void quit();
unsigned int get_address(unsigned int opcode_index, unsigned int value);
unsigned int get_byte(unsigned int address);
void set_byte(unsigned int address, unsigned int byte);
unsigned int get_register(unsigned int reg);
void set_register(unsigned int reg, unsigned int value);
void inc_register(unsigned int reg, int value);
void dec_register(unsigned int reg, int value);
void inc_cycles(unsigned int count);
unsigned int get_flag(unsigned int flag);
void set_flag(unsigned int flag, unsigned int value);
void stack_push(unsigned int value);
unsigned int stack_pull();

/* SIMULATE INSTRUCTIONS */
void do_aac(unsigned int opcode_index, unsigned int value);
void do_aax(unsigned int opcode_index, unsigned int value);
void do_adc(unsigned int opcode_index, unsigned int value);
void do_and(unsigned int opcode_index, unsigned int value);
void do_arr(unsigned int opcode_index, unsigned int value);
void do_asl(unsigned int opcode_index, unsigned int value);
void do_asr(unsigned int opcode_index, unsigned int value);
void do_atx(unsigned int opcode_index, unsigned int value);
void do_axa(unsigned int opcode_index, unsigned int value);
void do_axs(unsigned int opcode_index, unsigned int value);
void do_bcc(unsigned int opcode_index, unsigned int value);
void do_bcs(unsigned int opcode_index, unsigned int value);
void do_beq(unsigned int opcode_index, unsigned int value);
void do_bit(unsigned int opcode_index, unsigned int value);
void do_bmi(unsigned int opcode_index, unsigned int value);
void do_bne(unsigned int opcode_index, unsigned int value);
void do_bpl(unsigned int opcode_index, unsigned int value);
void do_brk(unsigned int opcode_index, unsigned int value);
void do_bvc(unsigned int opcode_index, unsigned int value);
void do_bvs(unsigned int opcode_index, unsigned int value);
void do_clc(unsigned int opcode_index, unsigned int value);
void do_cld(unsigned int opcode_index, unsigned int value);
void do_cli(unsigned int opcode_index, unsigned int value);
void do_clv(unsigned int opcode_index, unsigned int value);
void do_cmp(unsigned int opcode_index, unsigned int value);
void do_cpx(unsigned int opcode_index, unsigned int value);
void do_cpy(unsigned int opcode_index, unsigned int value);
void do_dcp(unsigned int opcode_index, unsigned int value);
void do_dec(unsigned int opcode_index, unsigned int value);
void do_dex(unsigned int opcode_index, unsigned int value);
void do_dey(unsigned int opcode_index, unsigned int value);
void do_dop(unsigned int opcode_index, unsigned int value);
void do_eor(unsigned int opcode_index, unsigned int value);
void do_inc(unsigned int opcode_index, unsigned int value);
void do_inx(unsigned int opcode_index, unsigned int value);
void do_iny(unsigned int opcode_index, unsigned int value);
void do_isc(unsigned int opcode_index, unsigned int value);
void do_jmp(unsigned int opcode_index, unsigned int value);
void do_jsr(unsigned int opcode_index, unsigned int value);
void do_kil(unsigned int opcode_index, unsigned int value);
void do_lar(unsigned int opcode_index, unsigned int value);
void do_lax(unsigned int opcode_index, unsigned int value);
void do_lda(unsigned int opcode_index, unsigned int value);
void do_ldx(unsigned int opcode_index, unsigned int value);
void do_ldy(unsigned int opcode_index, unsigned int value);
void do_lsr(unsigned int opcode_index, unsigned int value);
void do_nop(unsigned int opcode_index, unsigned int value);
void do_ora(unsigned int opcode_index, unsigned int value);
void do_pha(unsigned int opcode_index, unsigned int value);
void do_php(unsigned int opcode_index, unsigned int value);
void do_pla(unsigned int opcode_index, unsigned int value);
void do_plp(unsigned int opcode_index, unsigned int value);
void do_rla(unsigned int opcode_index, unsigned int value);
void do_rol(unsigned int opcode_index, unsigned int value);
void do_ror(unsigned int opcode_index, unsigned int value);
void do_rra(unsigned int opcode_index, unsigned int value);
void do_rti(unsigned int opcode_index, unsigned int value);
void do_rts(unsigned int opcode_index, unsigned int value);
void do_sbc(unsigned int opcode_index, unsigned int value);
void do_sec(unsigned int opcode_index, unsigned int value);
void do_sed(unsigned int opcode_index, unsigned int value);
void do_sei(unsigned int opcode_index, unsigned int value);
void do_slo(unsigned int opcode_index, unsigned int value);
void do_sre(unsigned int opcode_index, unsigned int value);
void do_sta(unsigned int opcode_index, unsigned int value);
void do_stx(unsigned int opcode_index, unsigned int value);
void do_sty(unsigned int opcode_index, unsigned int value);
void do_sxa(unsigned int opcode_index, unsigned int value);
void do_sya(unsigned int opcode_index, unsigned int value);
void do_tax(unsigned int opcode_index, unsigned int value);
void do_tay(unsigned int opcode_index, unsigned int value);
void do_top(unsigned int opcode_index, unsigned int value);
void do_tsx(unsigned int opcode_index, unsigned int value);
void do_txa(unsigned int opcode_index, unsigned int value);
void do_txs(unsigned int opcode_index, unsigned int value);
void do_tya(unsigned int opcode_index, unsigned int value);
void do_xaa(unsigned int opcode_index, unsigned int value);
void do_xas(unsigned int opcode_index, unsigned int value);

/* UTILS */
int power(int x, int y);
int root(int x, int y);
unsigned int crc_32(unsigned int *buffer, unsigned int length);
int hex2int(char *hex);
int bin2int(char *bin);
int oct2int(char *oct);
int dec2int(char *dec);
unsigned int fgetu16_little(FILE *fp);
unsigned int fgetu16_big(FILE *fp);
unsigned int fgetu32_little(FILE *fp);
unsigned int fgetu32_big(FILE *fp);
int get_fullpath(char **path, char *string);
unsigned int load_file(char **data, char *filename);

/* LIST */
unsigned int output_list(char *filename);

/* ERRORS */
void error(const char *fmt, ...);
unsigned int error_exists();
unsigned int error_exit();
void error_free();
void yyerror(const char *fmt, ...);

#endif /* _NESSEMBLE_H */
