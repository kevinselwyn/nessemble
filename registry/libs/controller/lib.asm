.ifndef CONTROLLER_COUNT ; 1 controller by default
CONTROLLER_COUNT = 1
.endif

    JMP controller_guard

.if CONTROLLER_COUNT >= 1 ; if 1 or more controllers

controller1_read:
    ; latch controller
    LDA #$01
    STA $4016
    LDA #$00
    STA $4016

    LDX #$08
controller1_loop:
    LDA $4016        ; read controller 1 keypress
    LSR A            ; shift keypress into carry
    ROL <controller1 ; rotate carry into controller1
    DEX
    BNE controller1_loop ; loop through the rest of the buttons

    RTS

.endif

;;;;;;;;

.if CONTROLLER_COUNT == 2 ; if 2 controllers

controller2_read:
    ; latch controller
    LDA #$01
    STA $4016
    LDA #$00
    STA $4016

    LDX #$08
controller2_loop:
    LDA $4017        ; read controller 2 keypress
    LSR A            ; shift keypress into carry
    ROL <controller2 ; rotate carry into controller2
    DEX
    BNE controller2_loop ; loop through the rest of the buttons

    RTS

.endif

controller_guard:
