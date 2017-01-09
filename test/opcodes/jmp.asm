    JMP $5597
    JMP [$5597]

label:
    JMP label
    JMP [label]

CONSTANT_16 = $5597

    JMP CONSTANT_16
    JMP [CONSTANT_16]
