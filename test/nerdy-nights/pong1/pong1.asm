.inesprg 1
.ineschr 1
.inesmap 0
.inesmir 1

;;;;;;;;;;;;;;;;

.rsset $0000

gamestate   .rs 1
ballx       .rs 1
bally       .rs 1
ballup      .rs 1
balldown    .rs 1
ballleft    .rs 1
ballright   .rs 1
ballspeedx  .rs 1
ballspeedy  .rs 1
paddle1ytop .rs 1
paddle2ybot .rs 1
buttons1    .rs 1
buttons2    .rs 1
score1      .rs 1
score2      .rs 1

;;;;;;;;

STATETITLE     = $00
STATEPLAYING   = $01
STATEGAMEOVER  = $02

RIGHTWALL      = $F4
TOPWALL        = $20
BOTTOMWALL     = $E0
LEFTWALL       = $04

PADDLE1X       = $08
PADDLE2X       = $F0

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
    LDA palette, x
    STA $2007
    INX
    CPX #$20
    BNE LoadPalettesLoop

;;;;;;;;

    LDA #$01
    STA balldown
    STA ballright
    LDA #$00
    STA ballup
    STA ballleft

    LDA #$50
    STA bally

    LDA #$80
    STA ballx

    LDA #$02
    STA ballspeedx
    STA ballspeedy
    LDA #STATEPLAYING
    STA gamestate

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

    JSR DrawScore
    LDA #%10010000
    STA $2000
    LDA #%00011110
    STA $2001
    LDA #$00
    STA $2005
    STA $2005

;;;;;;;;

    JSR ReadController1
    JSR ReadController2

;;;;;;;;

GameEngine:
    LDA gamestate
    CMP #STATETITLE
    BEQ EngineTitle

    LDA gamestate
    CMP #STATEGAMEOVER
    BEQ EngineGameOver

    LDA gamestate
    CMP #STATEPLAYING
    BEQ EnginePlaying
GameEngineDone:

;;;;;;;;

    JSR UpdateSprites

    RTI

;;;;;;;;

EngineTitle:
    JMP GameEngineDone

EngineGameOver:
    JMP GameEngineDone

;;;;;;;;

EnginePlaying:

MoveBallRight:
    LDA ballright
    BEQ MoveBallRightDone

    LDA ballx
    CLC
    ADC ballspeedx
    STA ballx

    LDA ballx
    CMP #RIGHTWALL
    BCC MoveBallRightDone
    LDA #$00
    STA ballright
    LDA #$01
    STA ballleft
MoveBallRightDone:

;;;;;;;;

MoveBallLeft:
    LDA ballleft
    BEQ MoveBallLeftDone

    LDA ballx
    SEC
    SBC ballspeedx
    STA ballx

    LDA ballx
    CMP #LEFTWALL
    BCS MoveBallLeftDone
    LDA #$01
    STA ballright
    LDA #$00
    STA ballleft
MoveBallLeftDone:

;;;;;;;;

MoveBallUp:
    LDA ballup
    BEQ MoveBallUpDone

    LDA bally
    SEC
    SBC ballspeedy
    STA bally

    LDA bally
    CMP #TOPWALL
    BCS MoveBallUpDone
    LDA #$01
    STA balldown
    LDA #$00
    STA ballup
MoveBallUpDone:

;;;;;;;;

MoveBallDown:
    LDA balldown
    BEQ MoveBallDownDone

    LDA bally
    CLC
    ADC ballspeedy
    STA bally

    LDA bally
    CMP #BOTTOMWALL
    BCC MoveBallDownDone
    LDA #$00
    STA balldown
    LDA #$01
    STA ballup
MoveBallDownDone:

;;;;;;;;

MovePaddleUp:
MovePaddleUpDone:

MovePaddleDown:
MovePaddleDownDone:

CheckPaddleCollision:
CheckPaddleCollisionDone:

  JMP GameEngineDone

;;;;;;;;

UpdateSprites:
    LDA bally
    STA $0200

    LDA #$30
    STA $0201

    LDA #$00
    STA $0202

    LDA ballx
    STA $0203

    RTS

;;;;;;;;

DrawScore:
    RTS

;;;;;;;;

ReadController1:
    LDA #$01
    STA $4016
    LDA #$00
    STA $4016
    LDX #$08
ReadController1Loop:
    LDA $4016
    LSR A
    ROL buttons1
    DEX
    BNE ReadController1Loop
    RTS

;;;;;;;;

ReadController2:
    LDA #$01
    STA $4016
    LDA #$00
    STA $4016
    LDX #$08
ReadController2Loop:
    LDA $4017
    LSR A
    ROL buttons2
    DEX
    BNE ReadController2Loop
    RTS

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

;;;;;;;;

.org $FFFA

    .dw NMI
    .dw RESET
    .dw 0

;;;;;;;;;;;;;;;;

.chr0

  .incpng "pong1.png"
