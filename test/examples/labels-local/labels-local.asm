    LDX #$08
:
    DEX
    STX $0200
    CPX #$03
    BNE :-
    CPX #$02
    BEQ :+
    STX $0201
:
    BRK
