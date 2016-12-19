    LDA #$44
    LDA <$44
    LDA <$44, X
    LDA $4400
    LDA $4400, X
    LDA $4400, Y
    LDA ($44, X)
    LDA ($44), Y

label:
    LDA #label
    LDA <label
    LDA <label, X
    LDA label
    LDA label, X
    LDA label, Y
    LDA (label, X)
    LDA (label), Y

CONSTANT_8 = $44
CONSTANT_16 = $4400

    LDA #CONSTANT_8
    LDA <CONSTANT_8
    LDA <CONSTANT_8, X
    LDA CONSTANT_16
    LDA CONSTANT_16, X
    LDA CONSTANT_16, Y
    LDA (CONSTANT_8, X)
    LDA (CONSTANT_8), Y
