DEFINED = $01

.ifdef UNDEFINED
.db $12, $34
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
