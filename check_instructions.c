#include <check.h>
#include <stdlib.h>
#include "nessemble.h"

START_TEST(test_instructions_get_opcode_brk) {
    int opcode_id = get_opcode("BRK", MODE_IMPLIED);

    ck_assert_int_eq(opcode_id, 0x00);
    ck_assert_str_eq(opcodes[opcode_id].mnemonic, "BRK");
    ck_assert_int_eq(opcodes[opcode_id].mode, MODE_IMPLIED);
    ck_assert_int_eq(opcodes[opcode_id].opcode, 0x00);
    ck_assert_int_eq(opcodes[opcode_id].length, 1);
    ck_assert_int_eq(opcodes[opcode_id].timing, 7);
    ck_assert_int_eq(opcodes[opcode_id].meta, META_NONE);
}
END_TEST

START_TEST(test_instructions_get_opcode_lda) {
    int opcode_id = get_opcode("LDA", MODE_IMMEDIATE);

    ck_assert_int_eq(opcode_id, 0xA9);
    ck_assert_str_eq(opcodes[opcode_id].mnemonic, "LDA");
    ck_assert_int_eq(opcodes[opcode_id].mode, MODE_IMMEDIATE);
    ck_assert_int_eq(opcodes[opcode_id].opcode, 0xA9);
    ck_assert_int_eq(opcodes[opcode_id].length, 2);
    ck_assert_int_eq(opcodes[opcode_id].timing, 2);
    ck_assert_int_eq(opcodes[opcode_id].meta, META_NONE);
}
END_TEST

START_TEST(test_instructions_get_opcode_invalid) {
    int opcode_id = get_opcode("INVALID", MODE_IMPLIED);

    ck_assert_int_eq(opcode_id, -1);
}
END_TEST

Suite * instruction_suite(void) {
    Suite *s;
    TCase *tc_core;

    s = suite_create("Instructions");

    tc_core = tcase_create("Core");

    tcase_add_test(tc_core, test_instructions_get_opcode_brk);
    tcase_add_test(tc_core, test_instructions_get_opcode_lda);
    tcase_add_test(tc_core, test_instructions_get_opcode_invalid);

    suite_add_tcase(s, tc_core);

    return s;
}

int main(int argc, char *argv[]) {
    int number_failed;
    Suite *s;
    SRunner *sr;

    s = instruction_suite();
    sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
