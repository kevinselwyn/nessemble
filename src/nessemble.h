#ifndef _NESSEMBLE_H
#define _NESSEMBLE_H

#include <stdio.h>
#include <getopt.h>
#include <time.h>
#include <setjmp.h>
#include "http.h"

/*
 * DEFINES
 */

/* PLATFORMS */
#if defined(WIN32) || defined(_WIN32) || (__MINGW32__)
#define IS_WINDOWS
#define SEP         "\\"
#define SEP_CHAR    '\\'
#define STDOUT_FILE "CON"
#endif /* WIN32 || _WIN32 || __MINGW32__ */

#if defined(__linux__)
#define IS_LINUX
#define SEP         "/"
#define SEP_CHAR    '/'
#define STDOUT_FILE SEP "dev" SEP "stdout"
#endif /* __linux__ */

#if defined(__APPLE__)
#define IS_MAC
#define SEP         "/"
#define SEP_CHAR    '/'
#define STDOUT_FILE SEP "dev" SEP "stdout"
#endif /* __APPLE__ */

#if defined(__EMSCRIPTEN__)
#define IS_JAVASCRIPT
#define SEP         "/"
#define SEP_CHAR    '/'
#define STDOUT_FILE SEP "dev" SEP "stdout"
#endif /* __EMSCRIPTEN__ */

/* UNUSED */
#define UNUSED(x) (void)(x)

/* I18N */
#define _(STRING) translate(STRING)

/* PROGRAM */
#define PROGRAM_NAME         "nessemble"
#define PROGRAM_VERSION      "1.0.1"
#define PROGRAM_COPYRIGHT    "2017"
#define PROGRAM_AUTHOR       "Kevin Selwyn"
#define PROGRAM_AUTHOR_EMAIL "kevinselwyn@gmail.com"
#define PROGRAM_LANGUAGE     "en-US"
#define PROGRAM_REPO         "https://github.com/kevinselwyn/nessemble"
#define PROGRAM_ISSUES       PROGRAM_REPO "/issues"

/* BOOLEAN */
#ifndef TRUE
#define TRUE  1
#endif /* TRUE */
#ifndef FALSE
#define FALSE 0
#endif /* FALSE */

/* MATH */
#define PI             3.14159265358979323846
#define CRC_TABLE_SIZE 256

/* RETURN CODES */
#define RETURN_OK    0
#define RETURN_EPERM 1
#define RETURN_USAGE 129

/* MIMETYPES */
#define MIMETYPE_JSON "application/json"
#define MIMETYPE_ZIP  "application/tar+gzip"
#define MIMETYPE_TEXT "text/plain"

/* VARS */
#define MAX_INCLUDE_DEPTH 10
#define BANK_PRG          0x4000
#define BANK_CHR          0x2000
#define BANK_SIZE         0x4000
#define MAX_SYMBOLS       65536
#define MAX_MACROS        1024
#define MAX_BANKS         256
#define MAX_INTS          256
#define MAX_TEXTS         256
#define MAX_ARGS          10
#define MAX_NESTED_IFS    10
#define TRAINER_MAX       512
#define BUF_SIZE          256
#define BUF_GET_LINE      256

/* PATH */
#ifndef PATH_MAX
#define PATH_MAX          4096
#endif

/* CLI FLAGS */
#define FLAG_UNDOCUMENTED 0x01
#define FLAG_NES          0x02
#define FLAG_DISASSEMBLE  0x04
#define FLAG_REASSEMBLE   0x08
#define FLAG_SIMULATE     0x10
#define FLAG_CHECK        0x20
#define FLAG_COVERAGE     0x40

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

/* JSON */
#define JSON_TOKEN_MAX 128

/* LIST */
#define CONSTANTS       "[constants]"
#define LABELS          "[labels]"
#define MAX_LIST_COUNT  65536
#define MAX_LIST_LENGTH 256

/* CONFIG */
#define CONFIG_API_DEFAULT "http://www.nessemble.com/registry"
#define CONFIG_FILENAME    "config"
#define CONFIG_LINE_COUNT  10
#define CONFIG_LINE_LENGTH 256
#define CONFIG_TYPES       1

/* MACRO */
#define MAX_MACRO_NAME 1024
#define MAX_MACRO_TEXT 1024 * 1024

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
#define USAGE_FLAG_COUNT            15
#define USAGE_COMMAND_COUNT         16
#define SIMULATION_USAGE_FLAG_COUNT 16

/* ZIP */
#define TAR_BLOCK_SIZE 512
#define ZIP_BUF_SIZE   512 * 1024
#define ZIP_INSIZE     (1 << 16)
#define ZIP_OUTSIZE    (1 << 16)

/* PNG */
#define COLOR_COUNT_2BIT 0x04
#define COLOR_COUNT_FULL 0x40

/* MAYBE UNDEFINED */
#ifndef CLOCK_MONOTONIC
#define CLOCK_MONOTONIC 1
#endif /* CLOCK_MONOTONIC */

#ifndef EINVAL
#define EINVAL 22
#endif /* EINVAL */

#ifndef EOVERFLOW
#define EOVERFLOW 75
#endif /* EOVERFLOW */

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
    unsigned int value, type, bank;
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

struct option commandline_options[USAGE_FLAG_COUNT+1];

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
unsigned int arg_unique;

/* PSEUDO */
char *pseudoname;

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
unsigned int length_texts;
char *texts[MAX_TEXTS];

/* ERROR REPORTING */
jmp_buf error_jmp;
char linebuf[512];
char yycolno;

/* INES */
extern struct ines_header ines;

/* IO */
unsigned int *rom;
unsigned int *coverage;

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
unsigned int usage(char *exec);
void usage_simulate();
unsigned int version();
unsigned int license();

/* HOME */
unsigned int get_home(char **home);
unsigned int get_home_path(char **path, unsigned int num, ...);

/* COVERAGE */
unsigned int get_coverage();

/* REFERENCE */
unsigned int reference(unsigned int terms, ...);

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
unsigned int is_flag_reassemble();
unsigned int is_flag_simulate();
unsigned int is_flag_check();
unsigned int is_flag_coverage();

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
int get_symbol_local(int direction);
char *symbol_argu(char *name);
void add_constant(char *name, unsigned int value);
void add_label(char *name);
int has_label(char *name);

/* PSEUDO UTILS */
void pseudo_ascii(char *string, int offset);
void pseudo_checksum(unsigned int address);
void pseudo_chr(unsigned int index);
void pseudo_color();
void pseudo_db();
void pseudo_defchr();
void pseudo_dw();
void pseudo_else();
void pseudo_endenum();
void pseudo_endif();
void pseudo_enum(unsigned int value, unsigned int inc);
void pseudo_fill();
void pseudo_font();
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
void pseudo_incrle(char *string);
void pseudo_incwav(char *string);
void pseudo_lobytes();
void pseudo_macro(char *string);
void pseudo_org(unsigned int address);
void pseudo_out(char *string);
void pseudo_prg(unsigned int index);
void pseudo_random();
void pseudo_rsset(unsigned int address);
void pseudo_rs(char *label, int size);
void pseudo_segment(char *string);
void pseudo_custom(char *pseudo);
unsigned int pseudo_parse(char **exec, char *pseudo);

/* OPCODE UTILS */
int get_opcode(char *mnemonic, unsigned int mode);

/* MACRO UTILS */
void add_macro(char *name);
void end_macro();
void append_macro(char *text);
int get_macro(char *name);
char *argu_macro(char *name);

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
unsigned int disassemble(char *input, char *output, char *listname);

/* SIMULATE UTILS */
unsigned int simulate(char *input, char *recipe);
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

/* MATH */
unsigned int crc_32(unsigned int *buffer, unsigned int length);

/* UTILS */
void *nessemble_malloc(size_t size);
void *nessemble_realloc(void *ptr, size_t size);
void nessemble_free(void *ptr);
void nessemble_fclose(FILE *file);
void nessemble_uppercase(char *str);
char *nessemble_strdup(char *str);
const char *nessemble_strcasestr(const char *s1, const char *s2);
int nessemble_mkdir(const char *dirname, int mode);
int nessemble_rmdir(const char *path);
char *nessemble_getpass(const char *prompt);
char *nessemble_realpath(const char *path, char *resolved_path);
unsigned int is_stdout(char *filename);
int hex2int(char *hex);
int bin2int(char *bin);
int oct2int(char *oct);
int dec2int(char *dec);
int defchr2int(char *defchr);
unsigned int str2hash(char *string);
unsigned int base64enc(char **encoded, char *str);
unsigned int base64dec(char **decoded, size_t *decoded_len, char *str);
unsigned int fgetu16_little(FILE *fp);
unsigned int fgetu16_big(FILE *fp);
unsigned int fgetu32_little(FILE *fp);
unsigned int fgetu32_big(FILE *fp);
unsigned int getu16_little(char *str);
unsigned int getu16_big(char *str);
unsigned int getu32_little(char *str);
unsigned int getu32_big(char *str);
unsigned int get_fullpath(char **path, char *string);
unsigned int get_libpath(char **path, char *string);
unsigned int file_exists(char *filename);
unsigned int load_file(char **data, unsigned int *data_length, char *filename);
unsigned int tmp_save(FILE *file, char *filename);
void tmp_delete(char *filename);
char *get_line(char **buffer, char *prompt);
unsigned int parse_extension(char **extension, char *filename);
unsigned int qrcode(unsigned int size, char *data, size_t data_len);

/* API */
unsigned int api_user(char **url, char *endpoint);
unsigned int api_lib(char **url, char *lib);
unsigned int api_search(char **url, char *term);
unsigned int api_reference(char **url, char *endpoint);

/* LIST */
unsigned int output_list(char *filename);
unsigned int input_list(char *filename);

/* ERRORS */
void fatal(const char *fmt, ...);
void warning(const char *fmt, ...);
void error(const char *fmt, ...);
unsigned int error_exists();
unsigned int error_exit();
void error_free();
void yyerror(const char *fmt, ...);
void error_program_log(const char *fmt, ...);
void error_program_output(const char *fmt, ...);
void error_signal();

/* INIT */
unsigned int init(int optc, char *optv[]);

/* CONFIG */
unsigned int create_config();
unsigned int open_config(FILE **file, char **filename);
void close_config(FILE *file, char *filename);
unsigned int get_config(char **result, char *item);
unsigned int set_config(char *result, char *item);
unsigned int list_config(char **result);

/* REGISTRY */
unsigned int get_registry();
unsigned int set_registry(char *registry);
unsigned int lib_install(char *lib);
unsigned int lib_uninstall(char *lib);
unsigned int lib_publish(char *filename, char **package);
unsigned int lib_info(char *lib);
unsigned int lib_list();
unsigned int lib_search(char *term);

/* JSON */
unsigned int get_json_buffer(char **value, char *key, char *json);
unsigned int get_json_file(char **value, char *key, char *filename);
unsigned int get_json_url(char **value, char *key, char *filename);
unsigned int get_json_search(char *url, char *term);

/* ZIP */
unsigned int untar_list(char ***filenames, size_t *filenames_count, char *tar, unsigned int tar_length);
unsigned int untar(char **data, size_t *data_length, char *tar, unsigned int tar_length, char *filename);
unsigned int get_ungzip(char **data, size_t *data_length, char *buffer, unsigned int buffer_length);
unsigned int get_unzipped(char **data, size_t *data_length, char *url);

/* PAGER */
unsigned int pager_buffer(char *buffer);
unsigned int pager_file(char *filename);

/* USER */
unsigned int user_auth(http_t *request, char *token, char *method, char *route);
unsigned int user_create(int optc, char *optv[]);
unsigned int user_login(int optc, char *optv[]);
unsigned int user_logout();
unsigned int user_forgotpassword(int optc, char *optv[]);
unsigned int user_resetpassword(int optc, char *optv[]);

/* I18N */
void translate_init();
void translate_free();
char *translate(char *id);

/* SCRIPTING */
unsigned int install_scripts(char **install_path);
unsigned int scripting_cmd(char *exec);
unsigned int scripting_js(char *exec, char **output);
unsigned int scripting_lua(char *exec, char **output);
unsigned int scripting_scm(char *exec, char **output);
unsigned int scripting_so(char *exec, char **output);

/* HASH */
void hash(char **hex, char *data, size_t data_len);
void hmac(char **hex, char *key, size_t key_len, char *data, size_t data_len);

#endif /* _NESSEMBLE_H */
