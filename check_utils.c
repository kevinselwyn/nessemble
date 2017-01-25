#include <check.h>
#include <stdlib.h>
#include <unistd.h>
#include "nessemble.h"

int saved_stderr = 0;
char buffer[4096] = { 0 };

void stderr_fake() {
    saved_stderr = dup(STDERR_FILENO);
    fclose(stderr);
    stderr = fmemopen(buffer, sizeof(buffer), "w");
    setbuf(stderr, NULL);
}

void stderr_real() {
    dup2(saved_stderr, STDERR_FILENO);
}

START_TEST(test_utils_power) {
    ck_assert_int_eq(power(1, 1), 1);
    ck_assert_int_eq(power(2, 2), 4);
    ck_assert_int_eq(power(3, 3), 27);
    ck_assert_int_eq(power(4, 4), 256);
}
END_TEST

START_TEST(test_utils_root) {
    ck_assert_int_eq(root(16, 2), 4);
    ck_assert_int_eq(root(9, 2), 3);
    ck_assert_int_eq(root(4, 2), 2);
    ck_assert_int_eq(root(1, 1), 1);
}
END_TEST

START_TEST(test_utils_crc_32) {
    unsigned int test1[5] = { 0x54, 0x68, 0x65 };
    unsigned int test2[5] = { 0x71, 0x75, 0x69, 0x63, 0x6B };
    unsigned int test3[5] = { 0x62, 0x72, 0x6F, 0x77, 0x6E };
    unsigned int test4[5] = { 0x66, 0x6F, 0x78 };

    ck_assert_uint_eq(crc_32(test1, 3), 0x04082B06);
    ck_assert_uint_eq(crc_32(test2, 5), 0x8DC71ED7);
    ck_assert_uint_eq(crc_32(test3, 5), 0x6D199454);
    ck_assert_uint_eq(crc_32(test4, 3), 0x0FA0E0E6);
}
END_TEST

START_TEST(test_utils_hex2int) {
    ck_assert_int_eq(hex2int("01"), 1);
    ck_assert_int_eq(hex2int("A5"), 165);
    ck_assert_int_eq(hex2int("4444"), 17476);
}
END_TEST

START_TEST(test_utils_bin2int) {
    ck_assert_int_eq(bin2int("00000001"), 1);
    ck_assert_int_eq(bin2int("10100101"), 165);
    ck_assert_int_eq(bin2int("0100010001000100"), 17476);
}
END_TEST

START_TEST(test_utils_dec2int) {
    ck_assert_int_eq(dec2int("1"), 1);
    ck_assert_int_eq(dec2int("165"), 165);
    ck_assert_int_eq(dec2int("17476"), 17476);
}
END_TEST

START_TEST(test_utils_get_fullpath) {
    char *path = NULL, *string = "test";

    ck_assert_int_eq(get_fullpath(&path, string), 0);
    ck_assert_str_eq(string, "test");
}
END_TEST

START_TEST(test_utils_load_file) {
    size_t length = 0;
    char *data = NULL;

    length = load_file(&data, __FILE__);
    data[18] = '\0';

    ck_assert_int_gt(length, 1800);
    ck_assert_str_eq(data, "#include <check.h>");

    if (data) {
        free(data);
    }
}
END_TEST

START_TEST(test_utils_is_flag_undocumented) {
    int old_flags = flags;

    flags = 0;
    ck_assert_int_eq(is_flag_undocumented(), 0);

    flags |= FLAG_UNDOCUMENTED;
    ck_assert_int_eq(is_flag_undocumented(), 1);

    flags = old_flags;
}
END_TEST

START_TEST(test_utils_is_flag_nes) {
    int old_flags = flags;

    flags = 0;
    ck_assert_int_eq(is_flag_nes(), 0);

    flags |= FLAG_NES;
    ck_assert_int_eq(is_flag_nes(), 1);

    flags = old_flags;
}
END_TEST

START_TEST(test_utils_is_flag_disassemble) {
    int old_flags = flags;

    flags = 0;
    ck_assert_int_eq(is_flag_disassemble(), 0);

    flags |= FLAG_DISASSEMBLE;
    ck_assert_int_eq(is_flag_disassemble(), 1);

    flags = old_flags;
}
END_TEST

START_TEST(test_utils_is_flag_simulate) {
    int old_flags = flags;

    flags = 0;
    ck_assert_int_eq(is_flag_simulate(), 0);

    flags |= FLAG_SIMULATE;
    ck_assert_int_eq(is_flag_simulate(), 1);

    flags = old_flags;
}
END_TEST

START_TEST(test_utils_is_flag_check) {
    int old_flags = flags;

    flags = 0;
    ck_assert_int_eq(is_flag_check(), 0);

    flags |= FLAG_CHECK;
    ck_assert_int_eq(is_flag_check(), 1);

    flags = old_flags;
}
END_TEST

START_TEST(test_utils_is_segment_chr) {
    int old_segment_type = segment_type;

    segment_type = 0;
    ck_assert_int_eq(is_segment_chr(), 0);

    segment_type |= SEGMENT_CHR;
    ck_assert_int_eq(is_segment_chr(), 1);

    segment_type = old_segment_type;
}
END_TEST

START_TEST(test_utils_is_segment_prg) {
    int old_segment_type = segment_type;

    segment_type = 0;
    ck_assert_int_eq(is_segment_prg(), 0);

    segment_type |= SEGMENT_PRG;
    ck_assert_int_eq(is_segment_prg(), 1);

    segment_type = old_segment_type;
}
END_TEST

Suite * utils_suite(void) {
    Suite *s;
    TCase *tc_core;

    s = suite_create("Utils");

    tc_core = tcase_create("Core");

    tcase_add_test(tc_core, test_utils_power);
    tcase_add_test(tc_core, test_utils_root);
    tcase_add_test(tc_core, test_utils_crc_32);
    tcase_add_test(tc_core, test_utils_hex2int);
    tcase_add_test(tc_core, test_utils_bin2int);
    tcase_add_test(tc_core, test_utils_dec2int);
    tcase_add_test(tc_core, test_utils_load_file);
    tcase_add_test(tc_core, test_utils_is_flag_undocumented);
    tcase_add_test(tc_core, test_utils_is_flag_nes);
    tcase_add_test(tc_core, test_utils_is_flag_disassemble);
    tcase_add_test(tc_core, test_utils_is_flag_simulate);
    tcase_add_test(tc_core, test_utils_is_flag_check);
    tcase_add_test(tc_core, test_utils_is_segment_chr);
    tcase_add_test(tc_core, test_utils_is_segment_prg);

    suite_add_tcase(s, tc_core);

    return s;
}

int main(int argc, char *argv[]) {
    int number_failed;
    Suite *s;
    SRunner *sr;

    s = utils_suite();
    sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
