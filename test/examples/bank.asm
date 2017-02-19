.inesprg 2
.ineschr 1
.inesmap 0
.inesmir 0

.prg 0
label1:
    JMP label2

.prg 1
.org $FFFA

label2:
    JMP label1

.chr 0
    .db $12, $34
