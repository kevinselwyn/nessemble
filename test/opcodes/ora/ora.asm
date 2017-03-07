    ORA #$44
    ORA <$44
    ORA <$44, X
    ORA $4400
    ORA $4400, X
    ORA $4400, Y
    ORA [$44, X]
    ORA [$44], Y

label:
    ORA #label
    ORA <label
    ORA <label, X
    ORA label
    ORA label, X
    ORA label, Y
    ORA [label, X]
    ORA [label], Y

CONSTANT_8 = $44
CONSTANT_16 = $4400

    ORA #CONSTANT_8
    ORA <CONSTANT_8
    ORA <CONSTANT_8, X
    ORA CONSTANT_16
    ORA CONSTANT_16, X
    ORA CONSTANT_16, Y
    ORA [CONSTANT_8, X]
    ORA [CONSTANT_8], Y
