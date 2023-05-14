    LDX #$08
:
    DEX
    STX $0200
    CPX #$03
    BNE :-
    BCC :+
    CPX #$02
    BEQ :++
:
    STX $0201
    BEQ :--
:
    BRK
