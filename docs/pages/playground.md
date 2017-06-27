# Playground

Experiment with `nessemble`

<div class="nessemble-example" data-opts='{"bare":true,"download":true}'>    LDX #$08
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
</div>

## Syntax

Need help with the syntax? Read more [here](/syntax).
