.enum $0080

TEST_0 .rs 1
TEST_1 .rs 2
TEST_2 .rs 1

.endenum

.db TEST_0, TEST_1, TEST_2

.enum $0040, 2

TEST_3 .rs 1
TEST_4 .rs 2
TEST_5 .rs 1

.endenum

.db TEST_3, TEST_4, TEST_5
