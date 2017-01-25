.inesprg 2
.ineschr 1
.inesmap 0
.inesmir 1

;;;;;;;;;;;;;;;;

.prg 0

RESET:
    SEI
    CLD

    LDA #%00000001
    STA $4015

    LDA #%10111111
    STA $4000

    LDA #$C9
    STA $4002
    LDA #$00
    STA $4003

;;;;;;;;

forever:
    JMP forever

;;;;;;;;

NMI:
    RTI

;;;;;;;;

.org $FFFA

    .dw NMI
    .dw RESET
    .dw 0
