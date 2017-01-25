.inesprg 1
.ineschr 1
.inesmap 0
.inesmir 0

.prg 0
label:
    .db $01, $02
    JMP label

.org $FFFA
    .db $03, $04, $05, $06, $07, $08

.chr 0
    .db $09, $0A
