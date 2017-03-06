    EOR #$44
    EOR <$44
    EOR <$44, X
    EOR $4400
    EOR $4400, X
    EOR $4400, Y
    EOR [$44, X]
    EOR [$44], Y

label:
    EOR #label
    EOR <label
    EOR <label, X
    EOR label
    EOR label, X
    EOR label, Y
    EOR [label, X]
    EOR [label], Y

CONSTANT_8 = $44
CONSTANT_16 = $4400

    EOR #CONSTANT_8
    EOR <CONSTANT_8
    EOR <CONSTANT_8, X
    EOR CONSTANT_16
    EOR CONSTANT_16, X
    EOR CONSTANT_16, Y
    EOR [CONSTANT_8, X]
    EOR [CONSTANT_8], Y
