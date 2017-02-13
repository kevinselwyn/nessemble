# controller

Read from the game controllers

## Explanation

The keypress state of the NES controllers are read in serially, bit-by-bit, using a shift register.

The shift register must be latched (brought high and then low) by writing to $4016, and then reading from $4016 for controller 1 and $4017 for controller 2.

This library reads that data into a single variable for `controller1` and `controller2`.

## Variables

1 byte in the zeropage should be assigned to `controller1`
Optionally, an additional byte in the zeropage should be assigned to `controller2`

To set the number of controllers to read, define the `CONTROLLER_COUNT` constant as 1 or 2

## Example

```
CONTROLLER_COUNT = 2

;;;;;;;;

.rsset $0000

controller1 .rs 1
controller2 .rs 1

;;;;;;;;

.include <controller.asm>

;;;;;;;;

    JSR controller1_read

    LDA <controller1
    LDA <controller2
```
