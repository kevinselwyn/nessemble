# prng

Pseudo-random number generator

## Explanation

This library utilizes a linear feedback shift register to generate a 16-bit pseudo-random number.

Read more about it here: https://wiki.nesdev.com/w/index.php/Random_number_generator

## Variables

2 bytes in the zeropage should be assigned to `prng_seed`

This variable should be initialized to any value except $00.

## Methods

`prng` - Generate new pseudo-random value (should be called once per frame)
`prng_byte` - Load pseudo-random value into A register

## Example

```
.rsset $0000

prng_seed .rs 2

;;;;;;;;

.include <prng.h>

;;;;;;;;

    ; initialize seed
    LDA #$13
    STA seed
    LDA #$37
    STA seed+1

    ; run prng
    JSR prng

    ; load pseudo-random byte into A register
    JSR prng_byte

    ; save value
    STA <$12
```
