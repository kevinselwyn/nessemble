    LDA LOW($1234)
    LDA HIGH($1234)
    LDA #LOW($1234)
    LDA #HIGH($1234)

label:
    LDA LOW(label)
    LDA HIGH(label)

CONSTANT = $1234

    LDA #LOW(CONSTANT)
    LDA #HIGH(CONSTANT)
