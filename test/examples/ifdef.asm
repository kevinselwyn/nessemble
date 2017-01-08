DEFINED = $01

.ifdef UNDEFINED
.db $01, $02
.endif

.ifdef DEFINED
.db $10, $20
.endif
