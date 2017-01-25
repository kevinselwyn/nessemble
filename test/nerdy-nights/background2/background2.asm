.inesprg 1
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

vblankwait2:
    BIT $2002
    BPL vblankwait2

;;;;;;;;

LoadPalettes:
    LDA $2002
    LDA #$3F
    STA $2006
    LDA #$00
    STA $2006
    LDX #$00
LoadPalettesLoop:
    LDA palette, X
    STA $2007
    INX
    CPX #$20
    BNE LoadPalettesLoop

;;;;;;;;

LoadSprites:
    LDX #$00
LoadSpritesLoop:
    LDA sprites, X
    STA $0200, X
    INX
    CPX #$10
    BNE LoadSpritesLoop

;;;;;;;;

LoadBackground:
    LDA $2002
    LDA #$20
    STA $2006
    LDA #$00
    STA $2006
    LDX #$00
LoadBackgroundLoop:
    LDA background, X
    STA $2007
    INX
    CPX #$80
    BNE LoadBackgroundLoop

;;;;;;;;

LoadAttribute:
    LDA $2002
    LDA #$23
    STA $2006
    LDA #$C0
    STA $2006
    LDX #$00
LoadAttributeLoop:
    LDA attribute, X
    STA $2007
    INX
    CPX #$08
    BNE LoadAttributeLoop

;;;;;;;;

    LDA #%10010000
    STA $2000

    LDA #%00011110
    STA $2001

;;;;;;;;

Forever:
    JMP Forever

;;;;;;;;

NMI:
    LDA #$00
    STA $2003
    LDA #$02
    STA $4014

;;;;;;;;

LatchController:
    LDA #$01
    STA $4016
    LDA #$00
    STA $4016

;;;;;;;;

ReadA:
    LDA $4016
    AND #%00000001
    BEQ ReadADone
    LDA $0203
    CLC
    ADC #$01
    STA $0203
ReadADone:

;;;;;;;;

ReadB:
    LDA $4016
    AND #%00000001
    BEQ ReadBDone
    LDA $0203
    SEC
    SBC #$01
    STA $0203
ReadBDone:

;;;;;;;;

    LDA #%10010000
    STA $2000
    LDA #%00011110
    STA $2001
    LDA #$00
    STA $2005
    STA $2005

    RTI

;;;;;;;;

.org $E000

palette:
    .db $22,$29,$1A,$0F, $22,$36,$17,$0F, $22,$30,$21,$0F, $22,$27,$17,$0F
    .db $22,$1C,$15,$14, $22,$02,$38,$3C, $22,$1C,$15,$14, $22,$02,$38,$3C

sprites:
    .db $80, $32, $00, $80
    .db $80, $33, $00, $88
    .db $88, $34, $00, $80
    .db $88, $35, $00, $88


background:
    .db $24,$24,$24,$24, $24,$24,$24,$24, $24,$24,$24,$24, $24,$24,$24,$24
    .db $24,$24,$24,$24, $24,$24,$24,$24, $24,$24,$24,$24, $24,$24,$24,$24

    .db $24,$24,$24,$24, $24,$24,$24,$24, $24,$24,$24,$24, $24,$24,$24,$24
    .db $24,$24,$24,$24, $24,$24,$24,$24, $24,$24,$24,$24, $24,$24,$24,$24

    .db $24,$24,$24,$24, $45,$45,$24,$24, $45,$45,$45,$45, $45,$45,$24,$24
    .db $24,$24,$24,$24, $24,$24,$24,$24, $24,$24,$24,$24, $53,$54,$24,$24

    .db $24,$24,$24,$24, $47,$47,$24,$24, $47,$47,$47,$47, $47,$47,$24,$24
    .db $24,$24,$24,$24, $24,$24,$24,$24, $24,$24,$24,$24, $55,$56,$24,$24

attribute:
    .db %00000000, %00010000, %01010000, %00010000, %00000000, %00000000, %00000000, %00110000
    .db $24,$24,$24,$24, $47,$47,$24,$24, $47,$47,$47,$47, $47,$47,$24,$24, $24,$24,$24,$24, $24,$24,$24,$24, $24,$24,$24,$24, $55,$56,$24,$24

;;;;;;;;

.org $FFFA

    .dw NMI
    .dw RESET
    .dw 0

;;;;;;;;;;;;;;;;

.chr 0

    .incpng "background2.png"
