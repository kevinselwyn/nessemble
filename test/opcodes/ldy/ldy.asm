    LDY #$44
    LDY <$44
    LDY <$44, X
    LDY $4400
    LDY $4400, X

label:
    LDY #label
    LDY <label
    LDY <label, X
    LDY label
    LDY label, X

CONSTANT_8 = $44
CONSTANT_16 = $4400

    LDY #CONSTANT_8
    LDY <CONSTANT_8
    LDY <CONSTANT_8, X
    LDY CONSTANT_16
    LDY CONSTANT_16, X
