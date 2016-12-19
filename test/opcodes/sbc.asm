    SBC #$44
    SBC <$44
    SBC <$44, X
    SBC $4400
    SBC $4400, X
    SBC $4400, Y
    SBC ($44, X)
    SBC ($44), Y

label:
    SBC #label
    SBC <label
    SBC <label, X
    SBC label
    SBC label, X
    SBC label, Y
    SBC (label, X)
    SBC (label), Y

CONSTANT_8 = $44
CONSTANT_16 = $4400

    SBC #CONSTANT_8
    SBC <CONSTANT_8
    SBC <CONSTANT_8, X
    SBC CONSTANT_16
    SBC CONSTANT_16, X
    SBC CONSTANT_16, Y
    SBC (CONSTANT_8, X)
    SBC (CONSTANT_8), Y
