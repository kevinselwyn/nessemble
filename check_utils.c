#include <check.h>
#include <stdlib.h>
#include "nessemble.h"

START_TEST(test_utils_power) {
    ck_assert_int_eq(power(1, 1), 1);
    ck_assert_int_eq(power(2, 2), 4);
    ck_assert_int_eq(power(3, 3), 27);
    ck_assert_int_eq(power(4, 4), 256);
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

Suite * utils_suite(void) {
    Suite *s;
    TCase *tc_core;

    s = suite_create("Utils");

    tc_core = tcase_create("Core");

    tcase_add_test(tc_core, test_utils_power);
    tcase_add_test(tc_core, test_utils_hex2int);
    tcase_add_test(tc_core, test_utils_bin2int);
    tcase_add_test(tc_core, test_utils_dec2int);
    tcase_add_test(tc_core, test_utils_load_file);

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
