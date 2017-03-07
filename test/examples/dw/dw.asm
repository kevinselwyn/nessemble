.dw $1234
.dw $4566+1, $89AC-1
.dw $CDEE+1, $0124-1, $4567+1-1

CONSTANT = $1234
CONSTANT_MINUS_1 = $1233
CONSTANT_PLUS_1 = $1235

.dw CONSTANT
.dw CONSTANT_MINUS_1+1, CONSTANT_PLUS_1-1
.dw CONSTANT, CONSTANT, CONSTANT

label:

.dw label
.dw label, label
.dw label, label, label

.word $1234
.word $4567, $89AB
.word $CDEF, $0123, $4567
