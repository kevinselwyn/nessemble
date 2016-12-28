#include <check.h>
#include <stdlib.h>
#include <unistd.h>
#include "nessemble.h"

int saved_stdout = 0;
char buffer[4096] = { 0 };

void stdout_fake() {
    saved_stdout = dup(STDOUT_FILENO);
    fclose(stdout);
    stdout = fmemopen(buffer, sizeof(buffer), "w");
    setbuf(stdout, NULL);
}

void stdout_real() {
    dup2(saved_stdout, STDOUT_FILENO);
}

START_TEST(test_usage_usage) {
    stdout_fake();

    ck_assert_int_eq(usage("nessemble"), 129);
    ck_assert_str_eq(buffer, "Usage: nessemble [options] <infile.asm>\n\n  -o, --output <outfile.rom>   output file\n  -f, --format {NES,RAW}       output format\n  -u, --undocumented           use undocumented opcodes\n  -l, --list <listfile.txt>    generate list of labels and constants\n  -c, --check                  check syntax only\n  -d, --disassemble            disassemble infile\n  -s, --simulate <infile.rom>  start the simulator\n  -r, --recipe <recipe.txt>    recipe file for the simulator\n  -h, --help                   print this message\n");

    stdout_real();
}
END_TEST

Suite * usage_suite(void) {
    Suite *s;
    TCase *tc_core;

    s = suite_create("Usage");

    tc_core = tcase_create("Core");

    tcase_add_test(tc_core, test_usage_usage);

    suite_add_tcase(s, tc_core);

    return s;
}

int main(int argc, char *argv[]) {
    int number_failed;
    Suite *s;
    SRunner *sr;

    s = usage_suite();
    sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
