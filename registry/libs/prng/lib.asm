prng:
    ; save registers
    PHA
    TXA
    PHA

    LDX #$08
    LDA <prng_seed

    ; loop 8 times
prng_loop:
    ASL A
    ROL <prng_seed+1
    BCC prng_skip
    EOR #$2D
prng_skip:
    DEX
    BNE prng_loop
    STA <prng_seed

    PLA
    TAX
    PLA

    RTS

;;;;;;;;

prng_byte:
    LDA <prng_seed

    RTS
