    STY <$44
    STY <$44, X
    STY $4400

label:
    STY <label
    STY <label, X
    STY label

CONSTANT_8 = $44
CONSTANT_16 = $4400

    STY <CONSTANT_8
    STY <CONSTANT_8, X
    STY CONSTANT_16
