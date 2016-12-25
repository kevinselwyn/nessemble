#ifndef _NESSEMBLE_H
#define _NESSEMBLE_H

#include <stdio.h>

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
#define MAX_BANKS         256
#define MAX_INTS          256
#define TRAINER_MAX       512

#ifndef PATH_MAX
#define PATH_MAX          4096
#endif

/* CLI FLAGS */
#define FLAG_UNDOCUMENTED 0x01
#define FLAG_NES          0x02
#define FLAG_DISASSEMBLE  0x04
#define FLAG_SIMULATE     0x08

unsigned int flags;

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

struct opcode {
    char mnemonic[4];
    unsigned int mode, opcode, length, timing, meta;
    void (*func)(int, int);
};

struct opcode opcodes[OPCODE_COUNT];

/* BISON */
int yylineno;
int yylex();
int yyparse();
FILE *yyin;

/* SYMBOLS */
#define SYMBOL_LABEL    0x00
#define SYMBOL_CONSTANT 0x01
#define SYMBOL_RS       0x02

struct symbol {
    char *name;
    unsigned int value, type;
};

struct symbol symbols[1024];
unsigned int symbol_index;
unsigned int rsset;

/* SEGMENTS */
char *segment;
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

/* INES HEADER */
struct ines_header {
    unsigned int chr, map, mir, prg, trn;
};

/* INES */
struct ines_header ines;

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

/* MAIN */
int main(int argc, char *argv[]);

/* USAGE */
#define USAGE_FLAG_COUNT 7

struct usage_flag {
    char *invocation, *description;
};

struct usage_flag usage_flags[USAGE_FLAG_COUNT];

int usage(char *exec);

/* INCLUDE UTILS */
void include_file_pop();
void include_file_push(char *filename);

/* FLAG TESTS */
unsigned int is_flag_undocumented();
unsigned int is_flag_nes();
unsigned int is_flag_disassemble();
unsigned int is_flag_simulate();

/* SEGMENT TESTS */
unsigned int is_segment_chr();
unsigned int is_segment_prg();

/* ERROR REPORTING */
void yyerror(const char *fmt, ...);

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

/* PSEUDO UTILS */
void pseudo_ascii(char *string, int offset);
void pseudo_chr(int index);
void pseudo_db();
void pseudo_defchr();
void pseudo_dw();
void pseudo_hibytes();
void pseudo_ineschr(int value);
void pseudo_inesmap(int value);
void pseudo_inesmir(int value);
void pseudo_inesprg(int value);
void pseudo_inestrn(char *string);
void pseudo_incbin(char *string, int offset, int limit);
void pseudo_include(char *string);
void pseudo_incpng(char *string, int offset, int limit);
void pseudo_lobytes();
void pseudo_org(int address);
void pseudo_out(char *string);
void pseudo_prg(int index);
void pseudo_rsset(int address);
void pseudo_rs(char *label, int size);
void pseudo_segment(char *string);

/* OPCODE UTILS */
int get_opcode(char *mnemonic, unsigned int mode);

/* ASSEMBLY UTILS */
void assemble_absolute(char *mnemonic, unsigned int address);
void assemble_absolute_xy(char *mnemonic, unsigned int address, char reg);
void assemble_accumulator(char *mnemonic);
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
int repl(char *input);
void print_registers();
void load_registers(char *input);
void print_instruction();
int step();
void print_memory(char *input);
void load_goto(char *input);
unsigned int get_data(int mode, int value);

void do_aac(int opcode_index, int value);
void do_aax(int opcode_index, int value);
void do_adc(int opcode_index, int value);
void do_and(int opcode_index, int value);
void do_arr(int opcode_index, int value);
void do_asl(int opcode_index, int value);
void do_asr(int opcode_index, int value);
void do_atx(int opcode_index, int value);
void do_axa(int opcode_index, int value);
void do_axs(int opcode_index, int value);
void do_bcc(int opcode_index, int value);
void do_bcs(int opcode_index, int value);
void do_beq(int opcode_index, int value);
void do_bit(int opcode_index, int value);
void do_bmi(int opcode_index, int value);
void do_bne(int opcode_index, int value);
void do_bpl(int opcode_index, int value);
void do_brk(int opcode_index, int value);
void do_bvc(int opcode_index, int value);
void do_bvs(int opcode_index, int value);
void do_clc(int opcode_index, int value);
void do_cld(int opcode_index, int value);
void do_cli(int opcode_index, int value);
void do_clv(int opcode_index, int value);
void do_cmp(int opcode_index, int value);
void do_cpx(int opcode_index, int value);
void do_cpy(int opcode_index, int value);
void do_dcp(int opcode_index, int value);
void do_dec(int opcode_index, int value);
void do_dex(int opcode_index, int value);
void do_dey(int opcode_index, int value);
void do_dop(int opcode_index, int value);
void do_eor(int opcode_index, int value);
void do_inc(int opcode_index, int value);
void do_inx(int opcode_index, int value);
void do_iny(int opcode_index, int value);
void do_isc(int opcode_index, int value);
void do_jmp(int opcode_index, int value);
void do_jsr(int opcode_index, int value);
void do_kil(int opcode_index, int value);
void do_lar(int opcode_index, int value);
void do_lax(int opcode_index, int value);
void do_lda(int opcode_index, int value);
void do_ldx(int opcode_index, int value);
void do_ldy(int opcode_index, int value);
void do_lsr(int opcode_index, int value);
void do_nop(int opcode_index, int value);
void do_ora(int opcode_index, int value);
void do_pha(int opcode_index, int value);
void do_php(int opcode_index, int value);
void do_pla(int opcode_index, int value);
void do_plp(int opcode_index, int value);
void do_rla(int opcode_index, int value);
void do_rol(int opcode_index, int value);
void do_ror(int opcode_index, int value);
void do_rra(int opcode_index, int value);
void do_rti(int opcode_index, int value);
void do_rts(int opcode_index, int value);
void do_sbc(int opcode_index, int value);
void do_sec(int opcode_index, int value);
void do_sed(int opcode_index, int value);
void do_sei(int opcode_index, int value);
void do_slo(int opcode_index, int value);
void do_sre(int opcode_index, int value);
void do_sta(int opcode_index, int value);
void do_stx(int opcode_index, int value);
void do_sty(int opcode_index, int value);
void do_sxa(int opcode_index, int value);
void do_sya(int opcode_index, int value);
void do_tax(int opcode_index, int value);
void do_tay(int opcode_index, int value);
void do_top(int opcode_index, int value);
void do_tsx(int opcode_index, int value);
void do_txa(int opcode_index, int value);
void do_txs(int opcode_index, int value);
void do_tya(int opcode_index, int value);
void do_xaa(int opcode_index, int value);
void do_xas(int opcode_index, int value);

/* UTILS */
int power(int x, int y);
int parse2int(char *text);
int hex2int(char *hex);
int bin2int(char *bin);
int oct2int(char *oct);
int dec2int(char *dec);
size_t load_file(char **data, char *filename);

#endif /* _NESSEMBLE_H */
