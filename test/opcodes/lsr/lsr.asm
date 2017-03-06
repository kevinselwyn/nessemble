    LSR A
    LSR <$44
    LSR <$44, X
    LSR $4400
    LSR $4400, X

label:
    LSR A
    LSR <label
    LSR <label, X
    LSR label
    LSR label, X

CONSTANT_8 = $44
CONSTANT_16 = $4400

    LSR A
    LSR <CONSTANT_8
    LSR <CONSTANT_8, X
    LSR CONSTANT_16
    LSR CONSTANT_16, X
