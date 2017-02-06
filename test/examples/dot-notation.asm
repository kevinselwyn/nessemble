sprite = $0200
coord_x = $03
coord_y = $00

    LDA #$80
    STA sprite->coord_x
    STA sprite->coord_y
