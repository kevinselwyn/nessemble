    ASL A
    ASL <$44
    ASL <$44, X
    ASL $4400
    ASL $4400, X

label:
    ASL A
    ASL <label
    ASL <label, X
    ASL label
    ASL label, X

CONSTANT_8 = $44
CONSTANT_16 = $4400

    ASL A
    ASL <CONSTANT_8
    ASL <CONSTANT_8, X
    ASL CONSTANT_16
    ASL CONSTANT_16, X
