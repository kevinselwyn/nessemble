.inesprg 1
.ineschr 1
.inesmap 0
.inesmir 0

.prg0
label:
    .db $01, $02
    JMP label

.org $FFFA
    .db $03, $04, $05, $06, $07, $08

.chr0
    .db $09, $0A
