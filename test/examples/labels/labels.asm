    LDX #$08
decrement:
    DEX
    STX $0200
    CPX #$03
    BNE decrement
    CPX #$02
    BEQ done
    STX $0201
done:
    BRK
