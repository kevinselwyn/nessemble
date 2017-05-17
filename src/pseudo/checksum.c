#include <stdio.h>
#include "../nessemble.h"

void pseudo_checksum(unsigned int address) {
    unsigned int crc = 0, index = get_rom_index();

    if (pass == 2) {
        if (index < address) {
            yyerror(_("Checksums may only be performed on preceding data"));
        }

        crc = crc_32(rom + address, index - address);
    }

    write_byte((crc >> 24) & 0xFF);
    write_byte((crc >> 16) & 0xFF);
    write_byte((crc >> 8) & 0xFF);
    write_byte(crc & 0xFF);
}
