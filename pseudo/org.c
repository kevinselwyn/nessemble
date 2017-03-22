#include "../nessemble.h"

/**
 * .org pseudo instruction
 * @param {int} address - Organization address
 */
void pseudo_org(unsigned int address) {
    // TODO: add check for too high an address

    if (is_segment_prg() == TRUE) {
        if (ines.prg < 2) {
            if (address < 0xC000) {
                yyerror(_("Start address for PRG bank %d is 0xC000"), prg_index);
            }

            if (address >= 0xC000 + BANK_PRG) {
                yyerror(_("Address too high"));
            }

            prg_offsets[prg_index] = address - 0xC000;
        } else {
            if ((prg_index % 2) == 0) {
                if (address < 0x8000) {
                    yyerror(_("Start address for PRG bank %d is 0x8000"), prg_index);
                }

                if (address >= 0x8000 + BANK_PRG) {
                    yyerror(_("Address too high"));
                }
            } else {
                if (address < 0xC000) {
                    yyerror(_("Start address for PRG bank %d is 0xC000"), prg_index);
                }

                if (address >= 0xC000 + BANK_PRG) {
                    yyerror(_("Address too high"));
                }
            }

            prg_offsets[prg_index] = address - 0x8000 - ((prg_index % 2) * BANK_PRG);
        }
    }

    if (is_segment_chr() == TRUE) {
        if (address >= BANK_CHR) {
            yyerror(_("Address too high"));
        }

        chr_offsets[chr_index] = address;
    }
}
