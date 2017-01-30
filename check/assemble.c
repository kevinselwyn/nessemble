#include <check.h>
#include <stdlib.h>
#include "../nessemble.h"

START_TEST(test_assemble_end_pass) {
    pass = 0;
    end_pass();

    ck_assert_int_eq(pass, 1);
}
END_TEST

START_TEST(test_assemble_get_rom_index) {
    ck_assert_int_eq(get_rom_index(), 0);
}
END_TEST

START_TEST(test_assemble_get_address_offset) {
    ck_assert_int_eq(get_address_offset(), 0);
}
END_TEST

START_TEST(test_assemble_write_byte) {
    pass = 2;
    rom = (unsigned int *)malloc(sizeof(unsigned int) * 1);

    write_byte(0x44);
    ck_assert_int_eq(rom[0], 0x44);

    free(rom);
}
END_TEST;

START_TEST(test_assemble_add_symbol) {
    pass = 1;
    add_symbol("test", 0x44, SYMBOL_LABEL);

    ck_assert_str_eq(symbols[0].name, "test");
    ck_assert_int_eq(symbols[0].value, 0x44);
    ck_assert_int_eq(symbols[0].type, SYMBOL_LABEL);
}
END_TEST;

Suite * assemble_suite(void) {
    Suite *s;
    TCase *tc_core;

    s = suite_create("Assemble");

    tc_core = tcase_create("Core");

    tcase_add_test(tc_core, test_assemble_end_pass);
    tcase_add_test(tc_core, test_assemble_get_rom_index);
    tcase_add_test(tc_core, test_assemble_get_address_offset);
    tcase_add_test(tc_core, test_assemble_write_byte);
    tcase_add_test(tc_core, test_assemble_add_symbol);

    suite_add_tcase(s, tc_core);

    return s;
}

int main(int argc, char *argv[]) {
    int number_failed;
    Suite *s;
    SRunner *sr;

    s = assemble_suite();
    sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
