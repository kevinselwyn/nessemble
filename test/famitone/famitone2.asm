; FamiTone2 v1.12

; settings, uncomment or put them into your main program; the latter makes possible updates easier

FT_BASE_ADR    = $0300 ; page in the RAM used for FT2 variables, should be $xx00
FT_TEMP        = $00   ; 3 bytes in zeropage used by the library as a scratchpad
FT_DPCM_OFF    = $c000 ; $c000..$ffc0, 64-byte steps
FT_SFX_STREAMS = 4     ; number of sound effects PLAyed at once, 1..4

; FT_DPCM_ENABLE         ; undefine to exclude all DMC code
; FT_SFX_ENABLE          ; undefine to exclude all sound effects code
; FT_THREAD              ; undefine if you are calling sound effects from the same thread as the sound update call

; FT_PAL_SUPPORT         ; undefine to exclude PAL support
; FT_NTSC_SUPPORT        ; undefine to exclude NTSC support

; internal defines

.ifdef FT_PAL_SUPPORT
.ifdef FT_NTSC_SUPPORT
FT_PITCH_FIX           ; add PAL/NTSC pitch correction code only when both modes are enabled
.endif
.endif

FT_DPCM_PTR = (FT_DPCM_OFF & $3FFF) >> 6

; zero page variables

FT_TEMP_PTR   = FT_TEMP       ; word
FT_TEMP_PTR_L = FT_TEMP_PTR + 0
FT_TEMP_PTR_H = FT_TEMP_PTR + 1
FT_TEMP_VAR1  = FT_TEMP + 2

; envelope structure offsets, 5 bytes per envelope, grouped by variable type

FT_ENVELOPES_ALL   = 3 + 3 + 3 + 2 ; 3 for the pulse AND triangle channels, 2 for the noise channel
FT_ENV_STRUCT_SIZE = 5

FT_ENV_VALUE  = FT_BASE_ADR + (0 * FT_ENVELOPES_ALL)
FT_ENV_REPEAT = FT_BASE_ADR + (1 * FT_ENVELOPES_ALL)
FT_ENV_ADR_L  = FT_BASE_ADR + (2 * FT_ENVELOPES_ALL)
FT_ENV_ADR_H  = FT_BASE_ADR + (3 * FT_ENVELOPES_ALL)
FT_ENV_PTR    = FT_BASE_ADR + (4 * FT_ENVELOPES_ALL)

; channel structure offsets, 7 bytes per channel

FT_CHANNELS_ALL    = 5
FT_CHN_STRUCT_SIZE = 9

FT_CHN_PTR_L      = FT_BASE_ADR + (0 * FT_CHANNELS_ALL)
FT_CHN_PTR_H      = FT_BASE_ADR + (1 * FT_CHANNELS_ALL)
FT_CHN_NOTE       = FT_BASE_ADR + (2 * FT_CHANNELS_ALL)
FT_CHN_INSTRUMENT = FT_BASE_ADR + (3 * FT_CHANNELS_ALL)
FT_CHN_REPEAT     = FT_BASE_ADR + (4 * FT_CHANNELS_ALL)
FT_CHN_RETURN_L   = FT_BASE_ADR + (5 * FT_CHANNELS_ALL)
FT_CHN_RETURN_H   = FT_BASE_ADR + (6 * FT_CHANNELS_ALL)
FT_CHN_REF_LEN    = FT_BASE_ADR + (7 * FT_CHANNELS_ALL)
FT_CHN_DUTY       = FT_BASE_ADR + (8 * FT_CHANNELS_ALL)

; variables AND aliases

FT_ENVELOPES = FT_BASE_ADR
FT_CH1_ENVS  = FT_ENVELOPES + 0
FT_CH2_ENVS  = FT_ENVELOPES + 3
FT_CH3_ENVS  = FT_ENVELOPES + 6
FT_CH4_ENVS  = FT_ENVELOPES + 9

FT_CHANNELS = FT_ENVELOPES + (FT_ENVELOPES_ALL * FT_ENV_STRUCT_SIZE)
FT_CH1_VARS = FT_CHANNELS + 0
FT_CH2_VARS = FT_CHANNELS + 1
FT_CH3_VARS = FT_CHANNELS + 2
FT_CH4_VARS = FT_CHANNELS + 3
FT_CH5_VARS = FT_CHANNELS + 4

FT_CH1_NOTE = FT_CH1_VARS + LOW(FT_CHN_NOTE)
FT_CH2_NOTE = FT_CH2_VARS + LOW(FT_CHN_NOTE)
FT_CH3_NOTE = FT_CH3_VARS + LOW(FT_CHN_NOTE)
FT_CH4_NOTE = FT_CH4_VARS + LOW(FT_CHN_NOTE)
FT_CH5_NOTE = FT_CH5_VARS + LOW(FT_CHN_NOTE)

FT_CH1_INSTRUMENT = FT_CH1_VARS + LOW(FT_CHN_INSTRUMENT)
FT_CH2_INSTRUMENT = FT_CH2_VARS + LOW(FT_CHN_INSTRUMENT)
FT_CH3_INSTRUMENT = FT_CH3_VARS + LOW(FT_CHN_INSTRUMENT)
FT_CH4_INSTRUMENT = FT_CH4_VARS + LOW(FT_CHN_INSTRUMENT)
FT_CH5_INSTRUMENT = FT_CH5_VARS + LOW(FT_CHN_INSTRUMENT)

FT_CH1_DUTY = FT_CH1_VARS + LOW(FT_CHN_DUTY)
FT_CH2_DUTY = FT_CH2_VARS + LOW(FT_CHN_DUTY)
FT_CH3_DUTY = FT_CH3_VARS + LOW(FT_CHN_DUTY)
FT_CH4_DUTY = FT_CH4_VARS + LOW(FT_CHN_DUTY)
FT_CH5_DUTY = FT_CH5_VARS + LOW(FT_CHN_DUTY)

FT_CH1_VOLUME = FT_CH1_ENVS + LOW(FT_ENV_VALUE) + 0
FT_CH2_VOLUME = FT_CH2_ENVS + LOW(FT_ENV_VALUE) + 0
FT_CH3_VOLUME = FT_CH3_ENVS + LOW(FT_ENV_VALUE) + 0
FT_CH4_VOLUME = FT_CH4_ENVS + LOW(FT_ENV_VALUE) + 0

FT_CH1_NOTE_OFF = FT_CH1_ENVS + LOW(FT_ENV_VALUE) + 1
FT_CH2_NOTE_OFF = FT_CH2_ENVS + LOW(FT_ENV_VALUE) + 1
FT_CH3_NOTE_OFF = FT_CH3_ENVS + LOW(FT_ENV_VALUE) + 1
FT_CH4_NOTE_OFF = FT_CH4_ENVS + LOW(FT_ENV_VALUE) + 1

FT_CH1_PITCH_OFF = FT_CH1_ENVS + LOW(FT_ENV_VALUE) + 2
FT_CH2_PITCH_OFF = FT_CH2_ENVS + LOW(FT_ENV_VALUE) + 2
FT_CH3_PITCH_OFF = FT_CH3_ENVS + LOW(FT_ENV_VALUE) + 2

FT_VARS = FT_CHANNELS + (FT_CHANNELS_ALL * FT_CHN_STRUCT_SIZE)

FT_PAL_ADJUST   = FT_VARS + 0
FT_SONG_LIST_L  = FT_VARS + 1
FT_SONG_LIST_H  = FT_VARS + 2
FT_INSTRUMENT_L = FT_VARS + 3
FT_INSTRUMENT_H = FT_VARS + 4
FT_TEMPO_STEP_L = FT_VARS + 5
FT_TEMPO_STEP_H = FT_VARS + 6
FT_TEMPO_ACC_L  = FT_VARS + 7
FT_TEMPO_ACC_H  = FT_VARS + 8
FT_SONG_SPEED   = FT_CH5_INSTRUMENT
FT_PULSE1_PREV  = FT_CH3_DUTY
FT_PULSE2_PREV  = FT_CH5_DUTY
FT_DPCM_LIST_L  = FT_VARS + 9
FT_DPCM_LIST_H  = FT_VARS + 10
FT_DPCM_EFFECT  = FT_VARS + 11
FT_OUT_BUF      = FT_VARS + 12 ; 11 bytes

; sound effect stream variables, 2 bytes AND 15 bytes per stream
; when sound effects are disabled, this memory is not used

FT_SFX_ADR_L    = FT_VARS + 23
FT_SFX_ADR_H    = FT_VARS + 24
FT_SFX_BASE_ADR = FT_VARS + 25

FT_SFX_STRUCT_SIZE = 15
FT_SFX_REPEAT      = FT_SFX_BASE_ADR + 0
FT_SFX_PTR_L       = FT_SFX_BASE_ADR + 1
FT_SFX_PTR_H       = FT_SFX_BASE_ADR + 2
FT_SFX_OFF         = FT_SFX_BASE_ADR + 3
FT_SFX_BUF         = FT_SFX_BASE_ADR + 4 ; 11 bytes

; aliases for sound effect channels to use in user calls

FT_SFX_CH0 = FT_SFX_STRUCT_SIZE * 0
FT_SFX_CH1 = FT_SFX_STRUCT_SIZE * 1
FT_SFX_CH2 = FT_SFX_STRUCT_SIZE * 2
FT_SFX_CH3 = FT_SFX_STRUCT_SIZE * 3

; aliases for the APU registers

APU_PL1_VOL    = $4000
APU_PL1_SWEEP  = $4001
APU_PL1_LO     = $4002
APU_PL1_HI     = $4003
APU_PL2_VOL    = $4004
APU_PL2_SWEEP  = $4005
APU_PL2_LO     = $4006
APU_PL2_HI     = $4007
APU_TRI_LINEAR = $4008
APU_TRI_LO     = $400a
APU_TRI_HI     = $400b
APU_NOISE_VOL  = $400c
APU_NOISE_LO   = $400e
APU_NOISE_HI   = $400f
APU_DMC_FREQ   = $4010
APU_DMC_RAW    = $4011
APU_DMC_START  = $4012
APU_DMC_LEN    = $4013
APU_SND_CHN    = $4015

; aliases for the APU registers in the output buffer

.ifndef FT_SFX_ENABLE    ; if sound effects are disabled, write to the APU directly
FT_MR_PULSE1_V = APU_PL1_VOL
FT_MR_PULSE1_L = APU_PL1_LO
FT_MR_PULSE1_H = APU_PL1_HI
FT_MR_PULSE2_V = APU_PL2_VOL
FT_MR_PULSE2_L = APU_PL2_LO
FT_MR_PULSE2_H = APU_PL2_HI
FT_MR_TRI_V    = APU_TRI_LINEAR
FT_MR_TRI_L    = APU_TRI_LO
FT_MR_TRI_H    = APU_TRI_HI
FT_MR_NOISE_V  = APU_NOISE_VOL
FT_MR_NOISE_F  = APU_NOISE_LO
.else                   ; otherwise write to the output buffer
FT_MR_PULSE1_V = FT_OUT_BUF
FT_MR_PULSE1_L = FT_OUT_BUF + 1
FT_MR_PULSE1_H = FT_OUT_BUF + 2
FT_MR_PULSE2_V = FT_OUT_BUF + 3
FT_MR_PULSE2_L = FT_OUT_BUF + 4
FT_MR_PULSE2_H = FT_OUT_BUF + 5
FT_MR_TRI_V    = FT_OUT_BUF + 6
FT_MR_TRI_L    = FT_OUT_BUF + 7
FT_MR_TRI_H    = FT_OUT_BUF + 8
FT_MR_NOISE_V  = FT_OUT_BUF + 9
FT_MR_NOISE_F  = FT_OUT_BUF + 10
.endif

;------------------------------------------------------------------------------
; reset APU, initialize FamiTone
; in: A   0 for PAL, not 0 for NTSC
;     X,Y pointer to music data
;------------------------------------------------------------------------------

FamiToneInit:
    STX FT_SONG_LIST_L ; store music data pointer for further use
    STY FT_SONG_LIST_H
    STX <FT_TEMP_PTR_L
    STY <FT_TEMP_PTR_H

.ifdef FT_PITCH_FIX
    TAX                 ; set SZ flags for A
    BEQ .pal
    LDA #64

.pal:
.else
.ifdef FT_PAL_SUPPORT
    LDA #0
.endif
.ifdef FT_NTSC_SUPPORT
    LDA #64
.endif
.endif
    STA FT_PAL_ADJUST

    JSR FamiToneMusicStop ; initialize channels AND envelopes

    LDY #1
    LDA [FT_TEMP_PTR],y   ; get instrument list address
    STA FT_INSTRUMENT_L
    INY
    LDA [FT_TEMP_PTR],y
    STA FT_INSTRUMENT_H
    INY
    LDA [FT_TEMP_PTR],y   ; get sample list address
    STA FT_DPCM_LIST_L
    INY
    LDA [FT_TEMP_PTR],y
    STA FT_DPCM_LIST_H

    LDA #$FF              ; previous pulse period MSB, to not write it when not changed
    STA FT_PULSE1_PREV
    STA FT_PULSE2_PREV

    LDA #$0F              ; enable channels, stop DMC
    STA APU_SND_CHN
    LDA #$80              ; disable triangle length counter
    STA APU_TRI_LINEAR
    LDA #$00              ; load noise length
    STA APU_NOISE_HI

    LDA #$30              ; volumes to 0
    STA APU_PL1_VOL
    STA APU_PL2_VOL
    STA APU_NOISE_VOL
    LDA #$08              ; no sweep
    STA APU_PL1_SWEEP
    STA APU_PL2_SWEEP

    ; JMP FamiToneMusicStop

;------------------------------------------------------------------------------
; stop music that is currently PLAying, if any
; in: none
;------------------------------------------------------------------------------

FamiToneMusicStop:
    LDA #0
    STA FT_SONG_SPEED      ; stop music, reset pause flag
    STA FT_DPCM_EFFECT     ; no DPCM effect PLAying

    LDX #LOW(FT_CHANNELS)  ; initialize channel structures

.set_channels:
    LDA #0
    STA FT_CHN_REPEAT,x
    STA FT_CHN_INSTRUMENT,x
    STA FT_CHN_NOTE,x
    STA FT_CHN_REF_LEN,x
    LDA #$30
    STA FT_CHN_DUTY,x

    INX                    ; next channel
    CPX #LOW(FT_CHANNELS) + FT_CHANNELS_ALL
    BNE .set_channels

    LDX #LOW(FT_ENVELOPES) ; initialize all envelopes to the dummy envelope

.set_envelopes:
    LDA #LOW (_FT2DummyEnvelope)
    STA FT_ENV_ADR_L,x
    LDA #HIGH(_FT2DummyEnvelope)
    STA FT_ENV_ADR_H,x
    LDA #0
    STA FT_ENV_REPEAT,x
    STA FT_ENV_VALUE,x
    INX
    CPX #LOW(FT_ENVELOPES)+FT_ENVELOPES_ALL

    BNE .set_envelopes

    JMP FamiToneSampleStop

;------------------------------------------------------------------------------
; PLAy music
; in: A number of subsong
;------------------------------------------------------------------------------

FamiToneMusicPLAy:
    LDX FT_SONG_LIST_L
    STX <FT_TEMP_PTR_L
    LDX FT_SONG_LIST_H
    STX <FT_TEMP_PTR_H

    LDY #0
    CMP [FT_TEMP_PTR],y   ; check if there is such sub song
    BCS .skip

    ASL a                 ; multiply song number by 14
    STA <FT_TEMP_PTR_L    ; use pointer LSB as temp variable
    ASL a
    TAX
    ASL a
    ADC <FT_TEMP_PTR_L
    STX <FT_TEMP_PTR_L
    ADC <FT_TEMP_PTR_L
    ADC #5                ; add offset
    TAY

    LDA FT_SONG_LIST_L    ; restore pointer LSB
    STA <FT_TEMP_PTR_L

    JSR FamiToneMusicStop ; stop music, initialize channels AND envelopes

    LDX #LOW(FT_CHANNELS) ; initialize channel structures

.set_channels:
    LDA [FT_TEMP_PTR],y   ; read channel pointers
    STA FT_CHN_PTR_L,x
    INY
    LDA [FT_TEMP_PTR],y
    STA FT_CHN_PTR_H,x
    INY

    LDA #0
    STA FT_CHN_REPEAT,x
    STA FT_CHN_INSTRUMENT,x
    STA FT_CHN_NOTE,x
    STA FT_CHN_REF_LEN,x
    LDA #$30
    STA FT_CHN_DUTY,x

    INX                   ; next channel
    CPX #LOW(FT_CHANNELS)+FT_CHANNELS_ALL
    BNE .set_channels

    LDA FT_PAL_ADJUST     ; read tempo for PAL or NTSC
    BEQ .pal
    INY
    INY

.pal:
    LDA [FT_TEMP_PTR],y   ; read the tempo step
    STA FT_TEMPO_STEP_L
    INY
    LDA [FT_TEMP_PTR],y
    STA FT_TEMPO_STEP_H

    LDA #0                ; reset tempo accumulator
    STA FT_TEMPO_ACC_L
    LDA #6                ; default speed
    STA FT_TEMPO_ACC_H
    STA FT_SONG_SPEED     ; apply default speed, this also enables music

.skip:
    RTS

;------------------------------------------------------------------------------
; pause AND unpause current music
; in: A 0 or not 0 to PLAy or pause
;------------------------------------------------------------------------------

FamiToneMusicPause:
    TAX                   ; set SZ flags for A
    BEQ .unpause

.pause:
    JSR FamiToneSampleStop

    LDA #0                ; mute sound
    STA FT_CH1_VOLUME
    STA FT_CH2_VOLUME
    STA FT_CH3_VOLUME
    STA FT_CH4_VOLUME
    LDA FT_SONG_SPEED     ; set pause flag
    ORA #$80
    BNE .done

.unpause:
    LDA FT_SONG_SPEED     ; reset pause flag
    AND #$7F

.done:
    STA FT_SONG_SPEED

    RTS

;------------------------------------------------------------------------------
; update FamiTone STAte, should be called every NMI
; in: none
;------------------------------------------------------------------------------

FamiToneUpdate:
.ifdef FT_THREAD
    LDA FT_TEMP_PTR_L
    PHA
    LDA FT_TEMP_PTR_H
    PHA
.endif

    LDA FT_SONG_SPEED       ; speed 0 means that no music is PLAying currently
    BMI .pause              ; bit 7 set is the pause flag
    BNE .update

.pause:
    JMP .update_sound

.update:
    CLC                     ; update frame counter that considers speed, tempo, AND PAL/NTSC
    LDA FT_TEMPO_ACC_L
    ADC FT_TEMPO_STEP_L
    STA FT_TEMPO_ACC_L
    LDA FT_TEMPO_ACC_H
    ADC FT_TEMPO_STEP_H
    CMP FT_SONG_SPEED
    BCS .update_row         ; overflow, row update is needed
    STA FT_TEMPO_ACC_H      ; no row update, skip to the envelopes update
    JMP .update_envelopes

.update_row:
    SEC
    SBC FT_SONG_SPEED
    STA FT_TEMPO_ACC_H

    LDX #LOW(FT_CH1_VARS)   ; process channel 1
    JSR _FT2ChannelUpdate
    BCC .no_new_note1
    LDX #LOW(FT_CH1_ENVS)
    LDA FT_CH1_INSTRUMENT
    JSR _FT2SetInstrument
    STA FT_CH1_DUTY

.no_new_note1:
    LDX #LOW(FT_CH2_VARS)   ; process channel 2
    JSR _FT2ChannelUpdate
    BCC .no_new_note2
    LDX #LOW(FT_CH2_ENVS)
    LDA FT_CH2_INSTRUMENT
    JSR _FT2SetInstrument
    STA FT_CH2_DUTY

.no_new_note2:
    LDX #LOW(FT_CH3_VARS)   ; process channel 3
    JSR _FT2ChannelUpdate
    BCC .no_new_note3
    LDX #LOW(FT_CH3_ENVS)
    LDA FT_CH3_INSTRUMENT
    JSR _FT2SetInstrument

.no_new_note3:
    LDX #LOW(FT_CH4_VARS)   ; process channel 4
    JSR _FT2ChannelUpdate
    BCC .no_new_note4
    LDX #LOW(FT_CH4_ENVS)
    LDA FT_CH4_INSTRUMENT
    JSR _FT2SetInstrument
    STA FT_CH4_DUTY

.no_new_note4:
.ifdef FT_DPCM_ENABLE
    LDX #LOW(FT_CH5_VARS)   ; process channel 5
    JSR _FT2ChannelUpdate
    BCC .no_new_note5
    LDA FT_CH5_NOTE
    BNE .PLAy_sample
    JSR FamiToneSampleStop
    BNE .no_new_note5       ; A is non-zero after FamiToneSampleStop
.PLAy_sample:
    JSR FamiToneSamplePLAyM
.no_new_note5:
.endif

.update_envelopes:
    LDX #LOW(FT_ENVELOPES)  ; process 11 envelopes

.env_process:
    LDA FT_ENV_REPEAT,x     ; check envelope repeat counter
    BEQ .env_read           ; if it is zero, process envelope
    DEC FT_ENV_REPEAT,x     ; otherwise decrement the counter
    BNE .env_next

.env_read:
    LDA FT_ENV_ADR_L,x      ; load envelope data address into temp
    STA <FT_TEMP_PTR_L
    LDA FT_ENV_ADR_H,x
    STA <FT_TEMP_PTR_H
    LDY FT_ENV_PTR,x        ; load envelope pointer

.env_read_value:
    LDA [FT_TEMP_PTR],y     ; read a byte of the envelope data
    BPL .env_special        ; values below 128 used as a special code, loop or repeat
    CLC                     ; values above 128 are output value+192 (output values are signed -63..64)
    ADC #256-192
    STA FT_ENV_VALUE,x      ; store the output value
    INY                     ; advance the pointer
    BNE .env_next_store_ptr ; bra

.env_special:
    BNE .env_set_repeat     ; zero is the loop point, non-zero values used for the repeat counter
    INY                     ; advance the pointer
    LDA [FT_TEMP_PTR],y     ; read loop position
    TAY                     ; use loop position
    JMP .env_read_value     ; read next byte of the envelope

.env_set_repeat:
    INY
    STA FT_ENV_REPEAT,x     ; store the repeat counter value

.env_next_store_ptr:
    TYA                     ; store the envelope pointer
    STA FT_ENV_PTR,x

.env_next:
    INX                     ; next envelope

    CPX #LOW(FT_ENVELOPES)+FT_ENVELOPES_ALL
    BNE .env_process

.update_sound:
    ; convert envelope AND channel output data into APU register values in the output buffer

    LDA FT_CH1_NOTE
    BEQ .ch1cut
    CLC
    ADC FT_CH1_NOTE_OFF
.ifdef FT_PITCH_FIX
    ORA FT_PAL_ADJUST
.endif
    TAX
    LDA FT_CH1_PITCH_OFF
    TAY
    ADC _FT2NoteTableLSB,x
    STA FT_MR_PULSE1_L
    TYA                     ; sign extension for the pitch offset
    ORA #$7F
    BMI .ch1sign
    LDA #0

.ch1sign:
    ADC _FT2NoteTableMSB,x

.ifndef FT_SFX_ENABLE
    CMP FT_PULSE1_PREV
    BEQ .ch1prev
    STA FT_PULSE1_PREV
.endif

    STA FT_MR_PULSE1_H

.ch1prev:
    LDA FT_CH1_VOLUME

.ch1cut:
    ORA FT_CH1_DUTY
    STA FT_MR_PULSE1_V

    LDA FT_CH2_NOTE
    BEQ .ch2cut
    CLC
    ADC FT_CH2_NOTE_OFF
.ifdef FT_PITCH_FIX
    ORA FT_PAL_ADJUST
.endif
    TAX
    LDA FT_CH2_PITCH_OFF
    TAY
    ADC _FT2NoteTableLSB,x
    STA FT_MR_PULSE2_L
    TYA
    ORA #$7f
    BMI .ch2sign
    LDA #0

.ch2sign:
    ADC _FT2NoteTableMSB,x

.ifndef FT_SFX_ENABLE
    CMP FT_PULSE2_PREV
    BEQ .ch2prev
    STA FT_PULSE2_PREV
.endif

    STA FT_MR_PULSE2_H
.ch2prev:
    LDA FT_CH2_VOLUME
.ch2cut:
    ORA FT_CH2_DUTY
    STA FT_MR_PULSE2_V

    LDA FT_CH3_NOTE
    BEQ .ch3cut
    CLC
    ADC FT_CH3_NOTE_OFF
.ifdef FT_PITCH_FIX
    ORA FT_PAL_ADJUST
.endif
    TAX
    LDA FT_CH3_PITCH_OFF
    TAY
    ADC _FT2NoteTableLSB,x
    STA FT_MR_TRI_L
    TYA
    ORA #$7F
    BMI .ch3sign
    LDA #0

.ch3sign:
    ADC _FT2NoteTableMSB,x
    STA FT_MR_TRI_H
    LDA FT_CH3_VOLUME

.ch3cut:
    ORA #$80
    STA FT_MR_TRI_V

    LDA FT_CH4_NOTE
    BEQ .ch4cut
    CLC
    ADC FT_CH4_NOTE_OFF
    AND #$0F
    EOR #$0F
    STA <FT_TEMP_VAR1
    LDA FT_CH4_DUTY
    ASL a
    AND #$80
    ORA <FT_TEMP_VAR1
    STA FT_MR_NOISE_F
    LDA FT_CH4_VOLUME

.ch4cut:
    ORA #$f0
    STA FT_MR_NOISE_V

.ifdef FT_SFX_ENABLE
    ; process all sound effect streams

.if FT_SFX_STREAMS>0
    LDX #FT_SFX_CH0
    JSR _FT2SfxUpdate
.endif
.if FT_SFX_STREAMS>1
    LDX #FT_SFX_CH1
    JSR _FT2SfxUpdate
.endif
.if FT_SFX_STREAMS>2
    LDX #FT_SFX_CH2
    JSR _FT2SfxUpdate
.endif
.if FT_SFX_STREAMS>3
    LDX #FT_SFX_CH3
    JSR _FT2SfxUpdate
.endif

    ; send data from the output buffer to the APU

    LDA FT_OUT_BUF          ; pulse 1 volume
    STA APU_PL1_VOL
    LDA FT_OUT_BUF+1        ; pulse 1 period LSB
    STA APU_PL1_LO
    LDA FT_OUT_BUF+2        ; pulse 1 period MSB, only applied when changed
    CMP FT_PULSE1_PREV
    BEQ .no_pulse1_upd
    STA FT_PULSE1_PREV
    STA APU_PL1_HI

.no_pulse1_upd:
    LDA FT_OUT_BUF+3        ; pulse 2 volume
    STA APU_PL2_VOL
    LDA FT_OUT_BUF+4        ; pulse 2 period LSB
    STA APU_PL2_LO
    LDA FT_OUT_BUF+5        ; pulse 2 period MSB, only applied when changed
    CMP FT_PULSE2_PREV
    BEQ .no_pulse2_upd
    STA FT_PULSE2_PREV
    STA APU_PL2_HI

.no_pulse2_upd:
    LDA FT_OUT_BUF+6        ; triangle volume (PLAys or not)
    STA APU_TRI_LINEAR
    LDA FT_OUT_BUF+7        ; triangle period LSB
    STA APU_TRI_LO
    LDA FT_OUT_BUF+8        ; triangle period MSB
    STA APU_TRI_HI

    LDA FT_OUT_BUF+9        ; noise volume
    STA APU_NOISE_VOL
    LDA FT_OUT_BUF+10       ; noise period
    STA APU_NOISE_LO
.endif

.ifdef FT_THREAD
    PLA
    STA FT_TEMP_PTR_H
    PLA
    STA FT_TEMP_PTR_L
.endif

    RTS

; internal routine, sets up envelopes of a channel according to current instrument
; in X envelope group offset, A instrument number

_FT2SetInstrument:
    ASL a                   ; instrument number is pre multiplied by 4
    TAY
    LDA FT_INSTRUMENT_H
    ADC #0                  ; use carry to extend range for 64 instruments
    STA <FT_TEMP_PTR_H
    LDA FT_INSTRUMENT_L
    STA <FT_TEMP_PTR_L

    LDA [FT_TEMP_PTR],y     ; duty cycle
    STA <FT_TEMP_VAR1
    INY

    LDA [FT_TEMP_PTR],y     ; instrument pointer LSB
    STA FT_ENV_ADR_L,x
    INY
    LDA [FT_TEMP_PTR],y     ; instrument pointer MSB
    INY
    STA FT_ENV_ADR_H,x
    INX                     ; next envelope

    LDA [FT_TEMP_PTR],y     ; instrument pointer LSB
    STA FT_ENV_ADR_L,x
    INY
    LDA [FT_TEMP_PTR],y     ; instrument pointer MSB
    STA FT_ENV_ADR_H,x

    LDA #0
    STA FT_ENV_REPEAT-1,x   ; reset env1 repeat counter
    STA FT_ENV_PTR-1,x      ; reset env1 pointer
    STA FT_ENV_REPEAT,x     ; reset env2 repeat counter
    STA FT_ENV_PTR,x        ; reset env2 pointer

    CPX #LOW(FT_CH4_ENVS)   ; noise channel has only two envelopes
    BCS .no_pitch

    INX                     ; next envelope
    INY
    STA FT_ENV_REPEAT,x     ; reset env3 repeat counter
    STA FT_ENV_PTR,x        ; reset env3 pointer
    LDA [FT_TEMP_PTR],y     ; instrument pointer LSB
    STA FT_ENV_ADR_L,x
    INY
    LDA [FT_TEMP_PTR],y     ; instrument pointer MSB
    STA FT_ENV_ADR_H,x

.no_pitch:
    LDA <FT_TEMP_VAR1

    RTS

; internal routine, parses channel note data

_FT2ChannelUpdate:
    LDA FT_CHN_REPEAT,x     ; check repeat counter
    BEQ .no_repeat
    DEC FT_CHN_REPEAT,x     ; DECrease repeat counter
    CLC                     ; no new note

    RTS

.no_repeat:
    LDA FT_CHN_PTR_L,x      ; load channel pointer into temp
    STA <FT_TEMP_PTR_L
    LDA FT_CHN_PTR_H,x
    STA <FT_TEMP_PTR_H

.no_repeat_r:
    LDY #0

.read_byte:
    LDA [FT_TEMP_PTR],y     ; read byte of the channel

    INC <FT_TEMP_PTR_L      ; advance pointer
    BNE .no_inc_ptr1
    INC <FT_TEMP_PTR_H

.no_inc_ptr1:
    ORA #0
    BMI .special_code       ; bit 7 0=note 1=special code

    LSR a                   ; bit 0 set means the note is followed by an empty row
    BCC .no_empty_row
    INC FT_CHN_REPEAT,x     ; set repeat counter to 1

.no_empty_row:
    STA FT_CHN_NOTE,x       ; store note code
    SEC                     ; new note flag is set
    BCS .done ;bra

.special_code:
    AND #$7f
    LSR a
    BCS .set_empty_rows
    ASL a
    ASL a
    STA FT_CHN_INSTRUMENT,x    ;store instrument number*4
    BCC .read_byte ;bra

.set_empty_rows:
    CMP #$3d
    BCC .set_repeat
    BEQ .set_speed
    CMP #$3e
    BEQ .set_loop

.set_reference:
    CLC                        ;remember return address+3
    LDA <FT_TEMP_PTR_L
    ADC #3
    STA FT_CHN_RETURN_L,x
    LDA <FT_TEMP_PTR_H
    ADC #0
    STA FT_CHN_RETURN_H,x
    LDA [FT_TEMP_PTR],y        ;read length of the reference (how many rows)
    STA FT_CHN_REF_LEN,x
    INY
    LDA [FT_TEMP_PTR],y        ;read 16-bit absolute address of the reference
    STA <FT_TEMP_VAR1        ;remember in temp
    INY
    LDA [FT_TEMP_PTR],y
    STA <FT_TEMP_PTR_H
    LDA <FT_TEMP_VAR1
    STA <FT_TEMP_PTR_L
    LDY #0
    JMP .read_byte

.set_speed:
    LDA [FT_TEMP_PTR],y
    STA FT_SONG_SPEED
    INC <FT_TEMP_PTR_L        ;advance pointer after reading the speed value
    BNE .read_byte
    INC <FT_TEMP_PTR_H
    BNE .read_byte ;bra

.set_loop:
    LDA [FT_TEMP_PTR],y
    STA <FT_TEMP_VAR1
    INY
    LDA [FT_TEMP_PTR],y
    STA <FT_TEMP_PTR_H
    LDA <FT_TEMP_VAR1
    STA <FT_TEMP_PTR_L
    DEY
    JMP .read_byte

.set_repeat:
    STA FT_CHN_REPEAT,x        ;set up repeat counter, carry is clear, no new note

.done:
    LDA FT_CHN_REF_LEN,x    ;check reference row counter
    BEQ .no_ref                ;if it is zero, there is no reference
    DEC FT_CHN_REF_LEN,x    ;DECrease row counter
    BNE .no_ref

    LDA FT_CHN_RETURN_L,x    ;end of a reference, return to previous pointer
    STA FT_CHN_PTR_L,x
    LDA FT_CHN_RETURN_H,x
    STA FT_CHN_PTR_H,x

    RTS

.no_ref:
    LDA <FT_TEMP_PTR_L
    STA FT_CHN_PTR_L,x
    LDA <FT_TEMP_PTR_H
    STA FT_CHN_PTR_H,x

    RTS

;------------------------------------------------------------------------------
; stop DPCM sample if it PLAys
;------------------------------------------------------------------------------

FamiToneSampleStop:
    LDA #%00001111
    STA APU_SND_CHN

    RTS

    .ifdef FT_DPCM_ENABLE

;------------------------------------------------------------------------------
; PLAy DPCM sample, used by music PLAyer, could be used externally
; in: A is number of a sample, 1..63
;------------------------------------------------------------------------------

FamiToneSamplePLAyM:        ;for music (low priority)
    LDX FT_DPCM_EFFECT
    BEQ _FT2SamplePLAy
    TAX
    LDA APU_SND_CHN
    AND #16
    BEQ .not_busy

    RTS

.not_busy:
    STA FT_DPCM_EFFECT
    TXA
    JMP _FT2SamplePLAy

;------------------------------------------------------------------------------
; PLAy DPCM sample with higher priority, for sound effects
; in: A is number of a sample, 1..63
;------------------------------------------------------------------------------

FamiToneSamplePLAy:
    LDX #1
    STX FT_DPCM_EFFECT

_FT2SamplePLAy:

    STA <FT_TEMP        ;sample number*3, offset in the sample table
    ASL a
    CLC
    ADC <FT_TEMP

    ADC FT_DPCM_LIST_L
    STA <FT_TEMP_PTR_L
    LDA #0
    ADC FT_DPCM_LIST_H
    STA <FT_TEMP_PTR_H

    LDA #%00001111            ;stop DPCM
    STA APU_SND_CHN

    LDY #0
    LDA [FT_TEMP_PTR],y        ;sample offset
    STA APU_DMC_START
    INY
    LDA [FT_TEMP_PTR],y        ;sample length
    STA APU_DMC_LEN
    INY
    LDA [FT_TEMP_PTR],y        ;pitch AND loop
    STA APU_DMC_FREQ

    LDA #32                    ;reset DAC counter
    STA APU_DMC_RAW
    LDA #%00011111            ;STArt DMC
    STA APU_SND_CHN

    RTS

.endif

.ifdef FT_SFX_ENABLE

;------------------------------------------------------------------------------
; init sound effects PLAyer, set pointer to data
; in: X,Y is address of sound effects data
;------------------------------------------------------------------------------

FamiToneSfxInit:
    STX <FT_TEMP_PTR_L
    STY <FT_TEMP_PTR_H

    LDY #0

.ifdef FT_PITCH_FIX

    LDA FT_PAL_ADJUST        ;add 2 to the sound list pointer for PAL
    BNE .ntsc
    INY
    INY
.ntsc:

.endif

    LDA [FT_TEMP_PTR],y        ;read AND store pointer to the effects list
    STA FT_SFX_ADR_L
    INY
    LDA [FT_TEMP_PTR],y
    STA FT_SFX_ADR_H

    LDX #FT_SFX_CH0            ;init all the streams

.set_channels:
    JSR _FT2SfxClearChannel
    TXA
    CLC
    ADC #FT_SFX_STRUCT_SIZE
    TAX
    CPX #FT_SFX_STRUCT_SIZE*FT_SFX_STREAMS
    BNE .set_channels

    RTS

;internal routine, clears output buffer of a sound effect
;in: A is 0
;    X is offset of sound effect stream

_FT2SfxClearChannel:
    LDA #0
    STA FT_SFX_PTR_H,x        ;this stops the effect
    STA FT_SFX_REPEAT,x
    STA FT_SFX_OFF,x
    STA FT_SFX_BUF+6,x        ;mute triangle
    LDA #$30
    STA FT_SFX_BUF+0,x        ;mute pulse1
    STA FT_SFX_BUF+3,x        ;mute pulse2
    STA FT_SFX_BUF+9,x        ;mute noise

    RTS

;------------------------------------------------------------------------------
; PLAy sound effect
; in: A is a number of the sound effect 0..127
;     X is offset of sound effect channel, should be FT_SFX_CH0..FT_SFX_CH3
;------------------------------------------------------------------------------

FamiToneSfxPLAy:
    ASL a                    ;get offset in the effects list
    TAY

    JSR _FT2SfxClearChannel    ;stops the effect if it PLAys

    LDA FT_SFX_ADR_L
    STA <FT_TEMP_PTR_L
    LDA FT_SFX_ADR_H
    STA <FT_TEMP_PTR_H

    LDA [FT_TEMP_PTR],y        ;read effect pointer from the table
    STA FT_SFX_PTR_L,x        ;store it
    INY
    LDA [FT_TEMP_PTR],y
    STA FT_SFX_PTR_H,x        ;this write enables the effect

    RTS


;internal routine, update one sound effect stream
;in: X is offset of sound effect stream

_FT2SfxUpdate:

    LDA FT_SFX_REPEAT,x        ;check if repeat counter is not zero
    BEQ .no_repeat
    DEC FT_SFX_REPEAT,x        ;DECrement AND return
    BNE .update_buf            ;just mix with output buffer

.no_repeat:
    LDA FT_SFX_PTR_H,x        ;check if MSB of the pointer is not zero
    BNE .sfx_active
    RTS                        ;return otherwise, no active effect

.sfx_active:
    STA <FT_TEMP_PTR_H        ;load effect pointer into temp
    LDA FT_SFX_PTR_L,x
    STA <FT_TEMP_PTR_L
    LDY FT_SFX_OFF,x
    CLC

.read_byte:
    LDA [FT_TEMP_PTR],y        ;read byte of effect
    BMI .get_data            ;if bit 7 is set, it is a register write
    BEQ .eof
    INY
    STA FT_SFX_REPEAT,x        ;if bit 7 is reset, it is number of repeats
    TYA
    STA FT_SFX_OFF,x
    JMP .update_buf

.get_data:
    INY
    STX <FT_TEMP_VAR1        ;it is a register write
    ADC <FT_TEMP_VAR1        ;get offset in the effect output buffer
    TAX
    LDA [FT_TEMP_PTR],y        ;read value
    INY
    STA FT_SFX_BUF-128,x    ;store into output buffer
    LDX <FT_TEMP_VAR1
    JMP .read_byte            ;AND read next byte

.eof:
    STA FT_SFX_PTR_H,x        ;mark channel as inactive

.update_buf:

    LDA FT_OUT_BUF            ;compare effect output buffer with main output buffer
    AND #$0f                ;if volume of pulse 1 of effect is higher than that of the
    STA <FT_TEMP_VAR1        ;main buffer, overwrite the main buffer value with the new one
    LDA FT_SFX_BUF+0,x
    AND #$0f
    CMP <FT_TEMP_VAR1
    BCC .no_pulse1
    LDA FT_SFX_BUF+0,x
    STA FT_OUT_BUF+0
    LDA FT_SFX_BUF+1,x
    STA FT_OUT_BUF+1
    LDA FT_SFX_BUF+2,x
    STA FT_OUT_BUF+2
.no_pulse1:

    LDA FT_OUT_BUF+3        ;same for pulse 2
    AND #$0f
    STA <FT_TEMP_VAR1
    LDA FT_SFX_BUF+3,x
    AND #$0f
    CMP <FT_TEMP_VAR1
    BCC .no_pulse2
    LDA FT_SFX_BUF+3,x
    STA FT_OUT_BUF+3
    LDA FT_SFX_BUF+4,x
    STA FT_OUT_BUF+4
    LDA FT_SFX_BUF+5,x
    STA FT_OUT_BUF+5
.no_pulse2:

    LDA FT_SFX_BUF+6,x        ;overwrite triangle of main output buffer if it is active
    BEQ .no_triangle
    STA FT_OUT_BUF+6
    LDA FT_SFX_BUF+7,x
    STA FT_OUT_BUF+7
    LDA FT_SFX_BUF+8,x
    STA FT_OUT_BUF+8
.no_triangle:

    LDA FT_OUT_BUF+9        ;same as for pulse 1 AND 2, but for noise
    AND #$0f
    STA <FT_TEMP_VAR1
    LDA FT_SFX_BUF+9,x
    AND #$0f
    CMP <FT_TEMP_VAR1
    BCC .no_noise
    LDA FT_SFX_BUF+9,x
    STA FT_OUT_BUF+9
    LDA FT_SFX_BUF+10,x
    STA FT_OUT_BUF+10
.no_noise:

    RTS

.endif

;dummy envelope used to initialize all channels with silence

_FT2DummyEnvelope:
    .db $c0,$00,$00

; PAL AND NTSC, 11-bit dividers
; rest note, then octaves 1-5, then three zeroes
; first 64 bytes are PAL, next 64 bytes are NTSC

_FT2NoteTableLSB:
.ifdef FT_PAL_SUPPORT
    .db $00,$33,$da,$86,$36,$eb,$a5,$62,$23,$e7,$af,$7a,$48,$19,$ec,$c2
    .db $9a,$75,$52,$30,$11,$f3,$d7,$bc,$a3,$8c,$75,$60,$4c,$3a,$28,$17
    .db $08,$f9,$eb,$dd,$d1,$c5,$ba,$af,$a5,$9c,$93,$8b,$83,$7c,$75,$6e
    .db $68,$62,$5c,$57,$52,$4d,$49,$45,$41,$3d,$3a,$36,$33,$30,$2d,$2b
.endif
.ifdef FT_NTSC_SUPPORT
    .db $00,$ad,$4d,$f2,$9d,$4c,$00,$b8,$74,$34,$f7,$be,$88,$56,$26,$f8
    .db $ce,$a5,$7f,$5b,$39,$19,$fb,$de,$c3,$aa,$92,$7b,$66,$52,$3f,$2d
    .db $1c,$0c,$fd,$ee,$e1,$d4,$c8,$bd,$b2,$a8,$9f,$96,$8d,$85,$7e,$76
    .db $70,$69,$63,$5e,$58,$53,$4f,$4a,$46,$42,$3e,$3a,$37,$34,$31,$2e
.endif
_FT2NoteTableMSB:
.ifdef FT_PAL_SUPPORT
    .db $00,$06,$05,$05,$05,$04,$04,$04,$04,$03,$03,$03,$03,$03,$02,$02
    .db $02,$02,$02,$02,$02,$01,$01,$01,$01,$01,$01,$01,$01,$01,$01,$01
    .db $01,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00
    .db $00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00
.endif
.ifdef FT_NTSC_SUPPORT
    .db $00,$06,$06,$05,$05,$05,$05,$04,$04,$04,$03,$03,$03,$03,$03,$02
    .db $02,$02,$02,$02,$02,$02,$01,$01,$01,$01,$01,$01,$01,$01,$01,$01
    .db $01,$01,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00
    .db $00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00
.endif
