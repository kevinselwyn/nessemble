#include <check.h>
#include <stdlib.h>
#include "../nessemble.h"

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

START_TEST(test_instructions_assemble_absolute) {
    int oldpass = pass,
        oldsegment_type = segment_type;

    if (!rom) {
        rom = (unsigned int *)malloc(sizeof(unsigned int) * 3);
    } else {
        rom = (unsigned int *)realloc(rom, sizeof(unsigned int) * 3);
    }

    pass = 2;
    segment_type |= SEGMENT_PRG;
    assemble_absolute("LDA", 0x1234);

    ck_assert_uint_eq(rom[0], 0xAD);
    ck_assert_uint_eq(rom[1], 0x34);
    ck_assert_uint_eq(rom[2], 0x12);

    pass = oldpass;
    segment_type = oldsegment_type;
}
END_TEST

START_TEST(test_instructions_assemble_absolute_xy) {
    int oldpass = pass,
        oldsegment_type = segment_type;

    if (!rom) {
        rom = (unsigned int *)malloc(sizeof(unsigned int) * 3);
    } else {
        rom = (unsigned int *)realloc(rom, sizeof(unsigned int) * 3);
    }

    pass = 2;
    segment_type |= SEGMENT_PRG;
    assemble_absolute_xy("LDA", 0x1234, 'X');

    ck_assert_uint_eq(rom[0], 0xBD);
    ck_assert_uint_eq(rom[1], 0x34);
    ck_assert_uint_eq(rom[2], 0x12);

    pass = oldpass;
    segment_type = oldsegment_type;
}
END_TEST

START_TEST(test_instructions_assemble_accumulator) {
    int oldpass = pass,
        oldsegment_type = segment_type;

    if (!rom) {
        rom = (unsigned int *)malloc(sizeof(unsigned int) * 1);
    } else {
        rom = (unsigned int *)realloc(rom, sizeof(unsigned int) * 1);
    }

    pass = 2;
    segment_type |= SEGMENT_PRG;
    assemble_accumulator("ROL", 'A');

    ck_assert_uint_eq(rom[0], 0x2A);

    pass = oldpass;
    segment_type = oldsegment_type;
}
END_TEST

START_TEST(test_instructions_assemble_implied) {
    int oldpass = pass,
        oldsegment_type = segment_type;

    if (!rom) {
        rom = (unsigned int *)malloc(sizeof(unsigned int) * 1);
    } else {
        rom = (unsigned int *)realloc(rom, sizeof(unsigned int) * 1);
    }

    pass = 2;
    segment_type |= SEGMENT_PRG;
    assemble_implied("TAX");

    ck_assert_uint_eq(rom[0], 0xAA);

    pass = oldpass;
    segment_type = oldsegment_type;
}
END_TEST

START_TEST(test_instructions_assemble_immediate) {
    int oldpass = pass,
        oldsegment_type = segment_type;

    if (!rom) {
        rom = (unsigned int *)malloc(sizeof(unsigned int) * 2);
    } else {
        rom = (unsigned int *)realloc(rom, sizeof(unsigned int) * 2);
    }

    pass = 2;
    segment_type |= SEGMENT_PRG;
    assemble_immediate("LDA", 0x12);

    ck_assert_uint_eq(rom[0], 0xA9);
    ck_assert_uint_eq(rom[1], 0x12);

    pass = oldpass;
    segment_type = oldsegment_type;
}
END_TEST

START_TEST(test_instructions_assemble_indirect) {
    int oldpass = pass,
        oldsegment_type = segment_type;

    if (!rom) {
        rom = (unsigned int *)malloc(sizeof(unsigned int) * 3);
    } else {
        rom = (unsigned int *)realloc(rom, sizeof(unsigned int) * 3);
    }

    pass = 2;
    segment_type |= SEGMENT_PRG;
    assemble_indirect("JMP", 0x1234);

    ck_assert_uint_eq(rom[0], 0x6C);
    ck_assert_uint_eq(rom[1], 0x34);
    ck_assert_uint_eq(rom[2], 0x12);

    pass = oldpass;
    segment_type = oldsegment_type;
}
END_TEST

START_TEST(test_instructions_assemble_indirect_xy) {
    int oldpass = pass,
        oldsegment_type = segment_type;

    if (!rom) {
        rom = (unsigned int *)malloc(sizeof(unsigned int) * 2);
    } else {
        rom = (unsigned int *)realloc(rom, sizeof(unsigned int) * 2);
    }

    pass = 2;
    segment_type |= SEGMENT_PRG;
    assemble_indirect_xy("LDA", 0x12, 'X');

    ck_assert_uint_eq(rom[0], 0xA1);
    ck_assert_uint_eq(rom[1], 0x12);

    pass = oldpass;
    segment_type = oldsegment_type;
}
END_TEST

START_TEST(test_instructions_assemble_relative) {
    int oldpass = pass,
        oldsegment_type = segment_type;

    if (!rom) {
        rom = (unsigned int *)malloc(sizeof(unsigned int) * 2);
    } else {
        rom = (unsigned int *)realloc(rom, sizeof(unsigned int) * 2);
    }

    pass = 2;
    segment_type |= SEGMENT_PRG;
    assemble_relative("BEQ", 0x20);

    ck_assert_uint_eq(rom[0], 0xF0);
    ck_assert_uint_eq(rom[1], 0x1E);

    pass = oldpass;
    segment_type = oldsegment_type;
}
END_TEST

START_TEST(test_instructions_assemble_zeropage) {
    int oldpass = pass,
        oldsegment_type = segment_type;

    if (!rom) {
        rom = (unsigned int *)malloc(sizeof(unsigned int) * 2);
    } else {
        rom = (unsigned int *)realloc(rom, sizeof(unsigned int) * 2);
    }

    pass = 2;
    segment_type |= SEGMENT_PRG;
    assemble_zeropage("LDA", 0x12);

    ck_assert_uint_eq(rom[0], 0xA5);
    ck_assert_uint_eq(rom[1], 0x12);

    pass = oldpass;
    segment_type = oldsegment_type;
}
END_TEST

START_TEST(test_instructions_assemble_zeropage_xy) {
    int oldpass = pass,
        oldsegment_type = segment_type;

    if (!rom) {
        rom = (unsigned int *)malloc(sizeof(unsigned int) * 2);
    } else {
        rom = (unsigned int *)realloc(rom, sizeof(unsigned int) * 2);
    }

    pass = 2;
    segment_type |= SEGMENT_PRG;
    assemble_zeropage_xy("LDA", 0x12, 'X');

    ck_assert_uint_eq(rom[0], 0xB5);
    ck_assert_uint_eq(rom[1], 0x12);

    pass = oldpass;
    segment_type = oldsegment_type;
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
    tcase_add_test(tc_core, test_instructions_assemble_absolute);
    tcase_add_test(tc_core, test_instructions_assemble_absolute_xy);
    tcase_add_test(tc_core, test_instructions_assemble_accumulator);
    tcase_add_test(tc_core, test_instructions_assemble_implied);
    tcase_add_test(tc_core, test_instructions_assemble_immediate);
    tcase_add_test(tc_core, test_instructions_assemble_indirect);
    tcase_add_test(tc_core, test_instructions_assemble_indirect_xy);
    tcase_add_test(tc_core, test_instructions_assemble_relative);
    tcase_add_test(tc_core, test_instructions_assemble_zeropage);
    tcase_add_test(tc_core, test_instructions_assemble_zeropage_xy);

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
