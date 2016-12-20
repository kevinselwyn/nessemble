#ifndef _NESSEMBLE_H
#define _NESSEMBLE_H

#define RETURN_OK    0
#define RETURN_EPERM 1
#define RETURN_USAGE 129

#define MAX_INCLUDE_DEPTH 10
#define BANK_PRG          0x4000
#define BANK_CHR          0x2000
#define BANK_SIZE         0x4000
#define MAX_BANKS         256
#define MAX_INTS          256
#define TRAINER_MAX       512

#define FLAG_UNDOCUMENTED 0x01
#define FLAG_NES          0x02

#define SEGMENT_CHR 1
#define SEGMENT_PRG 2

char cwd[1024];
char *cwd_path;

char linebuf[512];
char yycolno;

struct ines_header {
    int chr, map, mir, prg, trn;
};

int usage(char *exec);

void include_file_pop();
void include_file_push(char *filename);

int is_flag_undocumented();
int is_flag_nes();

int is_segment_chr();
int is_segment_prg();

void yyerror(const char *fmt, ...);

void assemble_feof();
void end_pass();

int get_offset();
void write_byte(unsigned int byte);

void add_symbol(char *name, int value, int type);
int get_symbol(char *name);
void add_constant(char *name, int value);
void add_label(char *name);

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

int get_opcode(char *mnemonic, int mode);

void assemble_absolute(char *mnemonic, int address);
void assemble_absolute_xy(char *mnemonic, int address, char reg);
void assemble_accumulator(char *mnemonic);
void assemble_implied(char *mnemonic);
void assemble_immediate(char *mnemonic, int value);
void assemble_indirect(char *mnemonic, int address);
void assemble_indirect_xy(char *mnemonic, int address, char reg);
void assemble_relative(char *mnemonic, int address);
void assemble_zeropage(char *mnemonic, int address);
void assemble_zeropage_xy(char *mnemonic, int address, char reg);

#endif /* _NESSEMBLE_H */
