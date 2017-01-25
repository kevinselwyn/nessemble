.inesprg 2
.ineschr 1
.inesmap 0
.inesmir 1

;;;;;;;;;;;;;;;;

.prg 0

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

    LDA #$80
    STA $2000

;;;;;;;;

enable_sound_channels:
    LDA #%00000111
    STA $4015

    LDA #%00111000
    STA $4000
    LDA #$C9
    STA $4002
    LDA #$00
    STA $4003

    LDA #%01110110
    STA $4004
    LDA #$A9
    STA $4006
    LDA #$00
    STA $4007

    LDA #$81
    STA $4008
    LDA #$42
    STA $400A
    LDA #$00
    STA $400B

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
