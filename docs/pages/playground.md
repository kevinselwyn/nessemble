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

> <div class="admonition note">
> <p class="admonition-title">Note</p>
> Need help with the syntax? Read more [here](/syntax).
> </div>