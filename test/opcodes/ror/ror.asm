    ROR A
    ROR <$44
    ROR <$44, X
    ROR $4400
    ROR $4400, X

label:
    ROR A
    ROR <label
    ROR <label, X
    ROR label
    ROR label, X

CONSTANT_8 = $44
CONSTANT_16 = $4400

    ROR A
    ROR <CONSTANT_8
    ROR <CONSTANT_8, X
    ROR CONSTANT_16
    ROR CONSTANT_16, X
