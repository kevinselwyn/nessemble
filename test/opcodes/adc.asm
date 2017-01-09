    ADC #$44
    ADC <$44
    ADC <$44, X
    ADC $4400
    ADC $4400, X
    ADC $4400, Y
    ADC [$44, X]
    ADC [$44], Y

label:
    ADC #label
    ADC <label
    ADC <label, X
    ADC label
    ADC label, X
    ADC label, Y
    ADC [label, X]
    ADC [label], Y

CONSTANT_8 = $44
CONSTANT_16 = $4400

    ADC #CONSTANT_8
    ADC <CONSTANT_8
    ADC <CONSTANT_8, X
    ADC CONSTANT_16
    ADC CONSTANT_16, X
    ADC CONSTANT_16, Y
    ADC [CONSTANT_8, X]
    ADC [CONSTANT_8], Y
