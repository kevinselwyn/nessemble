    STA <$44
    STA <$44, X
    STA $4400
    STA $4400, X
    STA $4400, Y
    STA ($44, X)
    STA ($44), Y

label:
    STA <label
    STA <label, X
    STA label
    STA label, X
    STA label, Y
    STA (label, X)
    STA (label), Y

CONSTANT_8 = $44
CONSTANT_16 = $4400

    STA <CONSTANT_8
    STA <CONSTANT_8, X
    STA CONSTANT_16
    STA CONSTANT_16, X
    STA CONSTANT_16, Y
    STA (CONSTANT_8, X)
    STA (CONSTANT_8), Y
