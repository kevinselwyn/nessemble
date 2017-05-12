#include <stdio.h>
#include "../nessemble.h"

void pseudo_checksum(unsigned int address) {
    unsigned int crc = 0;

    if (pass == 2) {
        crc = crc_32(rom + address, get_rom_index() - address);
    }

    write_byte((crc >> 24) & 0xFF);
    write_byte((crc >> 16) & 0xFF);
    write_byte((crc >> 8) & 0xFF);
    write_byte(crc & 0xFF);
}
