    ROL A
    ROL <$44
    ROL <$44, X
    ROL $4400
    ROL $4400, X

label:
    ROL A
    ROL <label
    ROL <label, X
    ROL label
    ROL label, X

CONSTANT_8 = $44
CONSTANT_16 = $4400

    ROL A
    ROL <CONSTANT_8
    ROL <CONSTANT_8, X
    ROL CONSTANT_16
    ROL CONSTANT_16, X
