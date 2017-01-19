.macrodef TEST_MACRO
    LDA #\1
    STA \2
.endm

.macro TEST_MACRO $01 $02
.macro TEST_MACRO $03 $04
