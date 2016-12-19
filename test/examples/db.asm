.db $12
.db $33+1, $54+1+1
.db $79-1, $9C-1-1, $BF-3+0

CONSTANT = $77

.db CONSTANT
.db CONSTANT, CONSTANT
.db CONSTANT, CONSTANT, CONSTANT

label:

.db label
.db label, label
.db label, label, label

.byte $12
.byte $34, $56
.byte $78, $9A, $BC
