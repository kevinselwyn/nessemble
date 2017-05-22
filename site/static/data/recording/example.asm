.inesprg 1
.ineschr 1
.inesmap 0
.inesmir 1

;;;;;;;;;;;;;;;;

.rsset $0000

scroll_y .rs 1

;;;;;;;;;;;;;;;;

.prg 0

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

LoadBackground:
    LDA $2002
    LDA #$20
    STA $2006
    LDA #$09
    STA $2006
    LDX #$00
LoadBackgroundLoop:
    LDA text, X
    STA $2007
    INX
    CPX #$0D
    BNE LoadBackgroundLoop

;;;;;;;;

LoadPalette:
    LDA $2002
    LDA #$3F
    STA $2006
    LDA #$00
    STA $2006
    LDX #$00
LoadPaletteLoop:
    LDA palette, X
    STA $2007
    INX
    CPX #$20
    BNE LoadPaletteLoop

;;;;;;;;

    LDA #%10000000
    STA $2000

    LDA #%00011110
    STA $2001

;;;;;;;;

    LDA #$00
    STA <scroll_y

;;;;;;;;

Forever:
    JMP Forever

;;;;;;;;

NMI:
    LDA #$00
    STA $2005

    INC <scroll_y
    LDA <scroll_y
    CMP #$F0
    BNE ScrollYDone
    LDA #$00
    STA <scroll_y
ScrollYDone:
    STA $2005

    RTI

;;;;;;;;

.org $E000

text:
    .ascii "Hello, World!"

palette:
    .color $000000, $000000, $000000, $000000
    .color $000000, $000000, $000000, $000000
    .color $000000, $000000, $000000, $000000
    .color $000000, $000000, $000000, $000000
    .color $FFFFFF, $FFFFFF, $FFFFFF, $FFFFFF
    .color $FFFFFF, $FFFFFF, $FFFFFF, $FFFFFF
    .color $FFFFFF, $FFFFFF, $FFFFFF, $FFFFFF
    .color $FFFFFF, $FFFFFF, $FFFFFF, $FFFFFF

;;;;;;;;

.org $FFFA

    .dw NMI
    .dw RESET
    .dw 0

;;;;;;;;;;;;;;;;

.chr 0

    .font 0, 127
