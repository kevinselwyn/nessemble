    CMP #$44
    CMP <$44
    CMP <$44, X
    CMP $4400
    CMP $4400, X
    CMP $4400, Y
    CMP [$44, X]
    CMP [$44], Y

label:
    CMP #label
    CMP <label
    CMP <label, X
    CMP label
    CMP label, X
    CMP label, Y
    CMP [label, X]
    CMP [label], Y

CONSTANT_8 = $44
CONSTANT_16 = $4400

    CMP #CONSTANT_8
    CMP <CONSTANT_8
    CMP <CONSTANT_8, X
    CMP CONSTANT_16
    CMP CONSTANT_16, X
    CMP CONSTANT_16, Y
    CMP [CONSTANT_8, X]
    CMP [CONSTANT_8], Y
