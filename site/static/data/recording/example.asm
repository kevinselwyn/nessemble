.inesprg 1
.ineschr 1
.inesmap 0
.inesmir 0

;;;;;;;;;;;;;;;;
; Reserved variables

.rsset $0000

direction .rs 1
counter   .rs 1

.rsset $0200

sprite .rs 4

;;;;;;;;;;;;;;;;
; Sprite constants

sprite_y    = $00
sprite_tile = $01
sprite_attr = $02
sprite_x    = $03

;;;;;;;;;;;;;;;;
; Register constants

PPUCTRL    = $2000
PPUMASK    = $2001
PPUSTATUS  = $2002
OAMADDR    = $2003
PPUSCROLL  = $2005
PPUADDR    = $2006
PPUDATA    = $2007
OAMDMA     = $4014

;;;;;;;;;;;;;;;;
; Direction constants

RIGHT = $00
DOWN  = $01
LEFT  = $02
UP    = $03

;;;;;;;;;;;;;;;;
; Position constants

DISTANCE  = $40
POS_X_MIN = $20
POS_X_MAX = $E0
POS_Y_MIN = $20
POS_Y_MAX = $D0

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

load_palettes:
    LDA PPUSTATUS
    LDA #$3F
    STA PPUADDR
    LDA #$00
    STA PPUADDR
    LDX #$00
load_palettes_loop:
    LDA palette, X
    STA PPUDATA
    INX
    CPX #$20
    BNE load_palettes_loop

;;;;;;;;

; Set sprite X/Y position
    LDA #$20
    STA sprite->sprite_x
    STA sprite->sprite_y

; Set sprite tile/attrs
    LDA #$00
    STA sprite->sprite_tile
    STA sprite->sprite_attr

;;;;;;;;

    LDA #%10010000
    STA PPUCTRL
    LDA #%00011110
    STA PPUMASK

; Init counter/direction
    LDA #$00
    STA <counter
    LDA #RIGHT
    STA <direction

;;;;;;;

Forever:
    JMP Forever

;;;;;;;;

NMI:
   
; Sprite DMA
    LDA #$00
    STA OAMADDR
    LDA #$02
    STA OAMDMA

; Set rendering
    LDA #%10010000
    STA PPUCTRL
    LDA #%00011110
    STA PPUMASK

; Reset scroll
    LDA #$00
    STA PPUSCROLL
    STA PPUSCROLL

; Handle right movement
right:
    LDA <direction
    CMP #RIGHT
    BNE right_done

    LDA <counter
    CMP #DISTANCE
    BEQ right_counter_done

    LDX <counter
    LDA ease_x, X
    STA sprite->sprite_x
    INC <counter

    JMP direction_done
right_counter_done:
    LDA #$00
    STA <counter
    LDA #DOWN
    STA <direction
    JMP direction_done
right_done:

; Handle down movement
down:
    LDA <direction
    CMP #DOWN
    BNE down_done

    LDA <counter
    CMP #DISTANCE
    BEQ down_counter_done

    LDY <counter
    LDA ease_y, Y
    STA sprite->sprite_y
    INC <counter

    JMP direction_done
down_counter_done:
    LDA #DISTANCE-1
    STA <counter
    LDA #LEFT
    STA <direction
    JMP direction_done
down_done:

; Handle left movement
left:
    LDA <direction
    CMP #LEFT
    BNE left_done

    LDA <counter
    CMP #$FF
    BEQ left_counter_done

    LDX <counter
    LDA ease_x, X
    STA sprite->sprite_x
    DEC <counter

    JMP direction_done
left_counter_done:
    LDA #DISTANCE-1
    STA <counter
    LDA #UP
    STA <direction
    JMP direction_done
left_done:

; Handle up movement
up:
    LDA <direction
    CMP #UP
    BNE up_done

    LDA <counter
    CMP #$FF
    BEQ up_counter_done

    LDY <counter
    LDA ease_y, Y
    STA sprite->sprite_y
    DEC <counter

    JMP direction_done
up_counter_done:
    LDA #$00
    STA <counter
    LDA #RIGHT
    STA <direction
    JMP direction_done
up_done:


direction_done:

    RTI

;;;;;;;;

.org $E000

; Color palette
palette:
    .color $000000, $000000, $000000, $000000
    .color $000000, $000000, $000000, $000000
    .color $000000, $000000, $000000, $000000
    .color $000000, $000000, $000000, $000000
    .color $000000, $FF0000, $FF0000, $FF0000
    .color $000000, $FF0000, $FF0000, $FF0000
    .color $000000, $FF0000, $FF0000, $FF0000
    .color $000000, $FF0000, $FF0000, $FF0000

; Easing data
ease_x:
    .ease "easeInOutCubic", POS_X_MIN, POS_X_MAX, DISTANCE
   
ease_y:
    .ease "easeInOutCubic", POS_Y_MIN, POS_Y_MAX, DISTANCE

.org $FFFA

    .dw NMI
    .dw RESET
    .dw 0

;;;;;;;;;;;;;;;;

.chr 0

; Square character tile
.defchr 33333333,
        30000003,
        30000003,
        30000003,
        30000003,
        30000003,
        30000003,
        33333333
