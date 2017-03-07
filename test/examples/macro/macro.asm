.macrodef TEST_MACRO
    LDA #\1
    STA <\2
.endm

.macrodef TEST_MACRO_2
    JMP \1
.endm

label:
    .macro TEST_MACRO, $01, $02
    .macro TEST_MACRO, $03, $04

    .macro TEST_MACRO_2, label
