.inesprg 1
.ineschr 1
.inesmap 0
.inesmir 1

;;;;;;;;;;;;;;;;

.prg0

RESET:
    SEI
    CLD
    LDX #$40
    STX $4017
    LDX #$FF
    TXS
    INX
    STX $2000
    STX $2001
    STX $4010

;;;;;;;;

vblankwait1:
    BIT $2002
    BPL vblankwait1

;;;;;;;;

clrmem:
    LDA #$00
    STA $0000, X
    STA $0100, X
    STA $0200, X
    STA $0400, X
    STA $0500, X
    STA $0600, X
    STA $0700, X
    LDA #$FE
    STA $0300, X
    INX
    BNE clrmem

;;;;;;;;

vblankwait2:
    BIT $2002
    BPL vblankwait2

;;;;;;;;

    LDA #%10000000
    STA $2001

;;;;;;;;

Forever:
    JMP Forever

;;;;;;;;

NMI:
    RTI

;;;;;;;;

.org $FFFA

    .dw NMI
    .dw RESET
    .dw 0

;;;;;;;;;;;;;;;;

.chr0

    .incpng "background.png"
