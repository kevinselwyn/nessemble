    LDX #$44
    LDX <$44
    LDX <$44, Y
    LDX $4400
    LDX $4400, Y

label:
    LDX #label
    LDX <label
    LDX <label, Y
    LDX label
    LDX label, Y

CONSTANT_8 = $44
CONSTANT_16 = $4400

    LDX #CONSTANT_8
    LDX <CONSTANT_8
    LDX <CONSTANT_8, Y
    LDX CONSTANT_16
    LDX CONSTANT_16, Y
