    INC <$44
    INC <$44, X
    INC $4400
    INC $4400, X

label:
    INC <label
    INC <label, X
    INC label
    INC label, X

CONSTANT_8 = $44
CONSTANT_16 = $4400

    INC <CONSTANT_8
    INC <CONSTANT_8, X
    INC CONSTANT_16
    INC CONSTANT_16, X
