    .db (2 / 2)

one:
    .db (1 + 1) * 1

two:
    .db (5 - 2)

three:
    .db 2 ** 2

four:
    .db (1 + 2 + 3 + 4) - (3 + 2)

five:

    .db (two / two)
    .db (one + one) * one
    .db (five - two)
    .db two ** two
    .db (one + two + three + four) - (three + two)

ONE   = $01
TWO   = $02
THREE = $03
FOUR  = $04
FIVE  = $05

    .db (TWO / TWO)
    .db (ONE + ONE) * ONE
    .db (FIVE - TWO)
    .db TWO ** TWO
    .db (ONE + TWO + THREE + FOUR) - (THREE + TWO)
