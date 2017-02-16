.ifndef rle_pointer
    .out "Please reserve 2 bytes in the zeropage for a `rle_pointer` variable"
.endif

    JMP rle_guard

rle:
    ; save registers
    PHA
    TYA
    PHA
    TXA
    PHA

    LDY #$00

rle_start:
    ; load first byte
    LDA [rle_pointer], Y

    ; if 0xFF, rle is done
    CMP #$FF
    BEQ rle_done

    ; if greater than 0x80, copy bytes directly
    ; if less than or equal to 0x80, repeat next byte A times
    CMP #$80
    BCC rle_repeat

    ; output byte string of length A
rle_length:
    AND #$7F
    TAX
    INY
    BNE rle_length_loop
    INC <rle_pointer+1

    ; loop through bytes
rle_length_loop:
    LDA [rle_pointer], Y
    STA $2007

    INY
    BNE rle_length_loop_continue
    INC <rle_pointer+1
rle_length_loop_continue:

    DEX
    BNE rle_length_loop
rle_lenth_done:

    ; read next byte
    JMP rle_start

    ; repeat next byte A number of times
rle_repeat:
    TAX
    INY
    BNE rle_repeat_continue
    INC <rle_pointer+1

rle_repeat_continue:
    LDA [rle_pointer], Y

    ; loop through bytes
rle_repeat_loop:
    STA $2007
    DEX
    BNE rle_repeat_loop

    INY
    BNE rle_repeat_done
    INC <rle_pointer+1
rle_repeat_done:

    ; read next byte
    JMP rle_start

rle_done:
    ; restore registers
    PLA
    TAX
    PLA
    TAY
    PLA

    RTS

rle_guard:
