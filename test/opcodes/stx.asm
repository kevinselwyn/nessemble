    STX <$44
    STX <$44, Y
    STX $4400

label:
    STX <label
    STX <label, Y
    STX label

CONSTANT_8 = $44
CONSTANT_16 = $4400

    STX <CONSTANT_8
    STX <CONSTANT_8, Y
    STX CONSTANT_16
