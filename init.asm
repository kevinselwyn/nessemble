vblankwait:
    BIT $2002
    BPL vblankwait
    RTS

;;;;;;;;

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

    JSR vblankwait

;;;;;;;;

clrmem:
    LDA #$00
    STA $0000, X
    STA $0100, X
    STA $0300, X
    STA $0400, X
    STA $0500, X
    STA $0600, X
    STA $0700, X
    LDA #$FE
    STA $0200, X
    INX
    BNE clrmem

;;;;;;;;

    JSR vblankwait

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
