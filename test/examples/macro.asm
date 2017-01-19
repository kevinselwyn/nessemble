.macrodef TEST_MACRO
    LDA #\1
    STA \2
    LDA #\3
    STA \3
.endm

.macro TEST_MACRO $01 $02
.macro TEST_MACRO $03 $04
