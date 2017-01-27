DEFINED = $01
NESTED  = $01

.ifdef UNDEFINED
.db $12, $34
.ifdef NESTED
.db $00, $00
.endif
.else
.db $56, $78
.endif

.ifdef DEFINED
.db $10, $20
.else
.db $30, $40
.endif

.if DEFINED == $01
.db $30
.endif
