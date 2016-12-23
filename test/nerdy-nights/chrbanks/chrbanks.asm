.inesprg 1
.ineschr 2
.inesmap 3
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
    LDA sprites, x
    STA $0200, x
    INX
    CPX #$20
    BNE LoadSpritesLoop

;;;;;;;;

    LDA #%10000000
    STA $2000

    LDA #%00010000
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

ReadSelect:
    LDA $4016
    AND #%00000001
    BEQ ReadSelectDone

    LDA #$00
    JSR Bankswitch
ReadSelectDone:

;;;;;;;;

ReadStart:
    LDA $4016
    AND #%00000001
    BEQ ReadStartDone

    LDA #$01
    JSR Bankswitch
ReadStartDone:

    RTI

;;;;;;;;

Bankswitch:
    TAX
    STA Bankvalues, X
    RTS

Bankvalues:
    .db $00, $01, $02, $03

;;;;;;;;

.org $E000

palette:
    .db $0F,$31,$32,$33, $34,$35,$36,$37, $38,$39,$3A,$3B, $3C,$3D,$3E,$0F
    .db $0F,$1C,$15,$14, $31,$02,$38,$3C, $0F,$1C,$15,$14, $31,$02,$38,$3C

sprites:
    .db $80, $32, $00, $80
    .db $80, $33, $00, $88
    .db $88, $34, $00, $80
    .db $88, $35, $00, $88

;;;;;;;;

.org $FFFA

    .dw NMI
    .dw RESET
    .dw 0

;;;;;;;;;;;;;;;;

.chr0

  .incpng "chrbanks0.png"

;;;;;;;;;;;;;;;;

.chr1

  .incpng "chrbanks1.png"
