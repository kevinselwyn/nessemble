#!/usr/bin/env python
# pylint: disable=C0301,R0914
"""Scrapes mapper data from http://wiki.nesdev.com"""

import sys
import argparse
import requests
import re
from bs4 import BeautifulSoup

mapper_list = {
    "000": "NROM",
    "001": "MMC1",
    "002": "UNROM",
    "003": "CNROM",
    "004": "MMC3",
    "005": "MMC5",
    "006": "FFE Rev. A",
    "007": "ANROM",
    "008": "unused",
    "009": "MMC2",
    "010": "MMC4",
    "011": "Color Dreams",
    "012": "REX DBZ 5",
    "013": "CPROM",
    "014": "REX SL-1632",
    "015": "100-in-1",
    "016": "BANDAI 24C02",
    "017": "FFE Rev. B",
    "018": "JALECO SS880006",
    "019": "Namcot 106",
    "020": "unused",
    "021": "Konami VRC2/VRC4 A",
    "022": "Konami VRC2/VRC4 B",
    "023": "Konami VRC2/VRC4 C",
    "024": "Konami VRC6 Rev. A",
    "025": "Konami VRC2/VRC4 D",
    "026": "Konami VRC6 Rev. B",
    "027": "CC-21 MI HUN CHE",
    "028": "unused",
    "029": "RET-CUFROM",
    "030": "UNROM 512",
    "031": "infiniteneslives-NSF",
    "032": "IREM G-101",
    "033": "TC0190FMC/TC0350FMR",
    "034": "IREM I-IM/BNROM",
    "035": "Wario Land 2",
    "036": "TXC Policeman",
    "037": "PAL-ZZ SMB/TETRIS/NWC",
    "038": "Bit Corp.",
    "039": "unused",
    "040": "SMB2j FDS",
    "041": "CALTRON 6-in-1",
    "042": "BIO MIRACLE FDS",
    "043": "FDS SMB2j LF36",
    "044": "MMC3 BMC PIRATE A",
    "045": "MMC3 BMC PIRATE B",
    "046": "RUMBLESTATION 15-in-1",
    "047": "NES-QJ SSVB/NWC",
    "048": "TAITO TCxxx",
    "049": "MMC3 BMC PIRATE C",
    "050": "SMB2j FDS Rev. A",
    "051": "11-in-1 BALL SERIES",
    "052": "MMC3 BMC PIRATE D",
    "053": "SUPERVISION 16-in-1",
    "054": "unused",
    "055": "unused",
    "056": "unused",
    "057": "SIMBPLE BMC PIRATE A",
    "058": "SIMBPLE BMC PIRATE B",
    "059": "unused",
    "060": "SIMBPLE BMC PIRATE C",
    "061": "20-in-1 KAISER Rev. A",
    "062": "700-in-1",
    "063": "unused",
    "064": "TENGEN RAMBO1",
    "065": "IREM-H3001",
    "066": "MHROM",
    "067": "SUNSOFT-FZII",
    "068": "Sunsoft Mapper #4",
    "069": "SUNSOFT-5/FME-7",
    "070": "BA KAMEN DISCRETE",
    "071": "CAMERICA BF9093",
    "072": "JALECO JF-17",
    "073": "KONAMI VRC3",
    "074": "TW MMC3+VRAM Rev. A",
    "075": "KONAMI VRC1",
    "076": "NAMCOT 108 Rev. A",
    "077": "IREM LROG017",
    "078": "Irem 74HC161/32",
    "079": "AVE/C&E/TXC BOARD",
    "080": "TAITO X1-005 Rev. A",
    "081": "unused",
    "082": "TAITO X1-017",
    "083": "YOKO VRC Rev. B",
    "084": "unused",
    "085": "KONAMI VRC7",
    "086": "JALECO JF-13",
    "087": "74*139/74 DISCRETE",
    "088": "NAMCO 3433",
    "089": "SUNSOFT-3",
    "090": "HUMMER/JY BOARD",
    "091": "EARLY HUMMER/JY BOARD",
    "092": "JALECO JF-19",
    "093": "SUNSOFT-3R",
    "094": "HVC-UN1ROM",
    "095": "NAMCOT 108 Rev. B",
    "096": "BANDAI OEKAKIDS",
    "097": "IREM TAM-S1",
    "098": "unused",
    "099": "VS Uni/Dual- system",
    "100": "unused",
    "101": "unused",
    "102": "unused",
    "103": "FDS DOKIDOKI FULL",
    "104": "unused",
    "105": "NES-EVENT NWC1990",
    "106": "SMB3 PIRATE A",
    "107": "MAGIC CORP A",
    "108": "FDS UNROM BOARD",
    "109": "unused",
    "110": "unused",
    "111": "unused",
    "112": "ASDER/NTDEC BOARD",
    "113": "HACKER/SACHEN BOARD",
    "114": "MMC3 SG PROT. A",
    "115": "MMC3 PIRATE A",
    "116": "MMC1/MMC3/VRC PIRATE",
    "117": "FUTURE MEDIA BOARD",
    "118": "TSKROM",
    "119": "NES-TQROM",
    "120": "FDS TOBIDASE",
    "121": "MMC3 PIRATE PROT. A",
    "122": "unused",
    "123": "MMC3 PIRATE H2288",
    "124": "unused",
    "125": "FDS LH32",
    "126": "unused",
    "127": "unused",
    "128": "unused",
    "129": "unused",
    "130": "unused",
    "131": "unused",
    "132": "TXC/MGENIUS 22111",
    "133": "SA72008",
    "134": "MMC3 BMC PIRATE",
    "135": "unused",
    "136": "TCU02",
    "137": "S8259D",
    "138": "S8259B",
    "139": "S8259C",
    "140": "JALECO JF-11/14",
    "141": "S8259A",
    "142": "UNLKS7032",
    "143": "TCA01",
    "144": "AGCI 50282",
    "145": "SA72007",
    "146": "SA0161M",
    "147": "TCU01",
    "148": "SA0037",
    "149": "SA0036",
    "150": "S74LS374N",
    "151": "unused",
    "152": "unused",
    "153": "BANDAI SRAM",
    "154": "unused",
    "155": "unused",
    "156": "unused",
    "157": "BANDAI BARCODE",
    "158": "unused",
    "159": "BANDAI 24C01",
    "160": "SA009",
    "161": "unused",
    "162": "unused",
    "163": "unused",
    "164": "unused",
    "165": "unused",
    "166": "SUBOR Rev. A",
    "167": "SUBOR Rev. B",
    "168": "unused",
    "169": "unused",
    "170": "unused",
    "171": "unused",
    "172": "unused",
    "173": "unused",
    "174": "unused",
    "175": "unused",
    "176": "BMCFK23C",
    "177": "unused",
    "178": "unused",
    "179": "unused",
    "180": "unused",
    "181": "unused",
    "182": "unused",
    "183": "unused",
    "184": "unused",
    "185": "unused",
    "186": "unused",
    "187": "unused",
    "188": "unused",
    "189": "unused",
    "190": "unused",
    "191": "unused",
    "192": "TW MMC3+VRAM Rev. B",
    "193": "NTDEC TC-112",
    "194": "TW MMC3+VRAM Rev. C",
    "195": "TW MMC3+VRAM Rev. D",
    "196": "unused",
    "197": "unused",
    "198": "TW MMC3+VRAM Rev. E",
    "199": "unused",
    "200": "unused",
    "201": "unused",
    "202": "unused",
    "203": "unused",
    "204": "unused",
    "205": "unused",
    "206": "NAMCOT 108 Rev. C",
    "207": "TAITO X1-005 Rev. B",
    "208": "unused",
    "209": "unused",
    "210": "unused",
    "211": "unused",
    "212": "unused",
    "213": "unused",
    "214": "unused",
    "215": "unused",
    "216": "unused",
    "217": "unused",
    "218": "unused",
    "219": "UNLA9746",
    "220": "Debug Mapper",
    "221": "UNLN625092",
    "222": "unused",
    "223": "unused",
    "224": "unused",
    "225": "unused",
    "226": "BMC 22+20-in-1",
    "227": "unused",
    "228": "unused",
    "229": "unused",
    "230": "BMC Contra+22-in-1",
    "231": "unused",
    "232": "BMC QUATTRO",
    "233": "BMC 22+20-in-1 RST",
    "234": "BMC MAXI",
    "235": "unused",
    "236": "unused",
    "237": "unused",
    "238": "UNL6035052",
    "239": "unused",
    "240": "unused",
    "241": "unused",
    "242": "unused",
    "243": "S74LS374NA",
    "244": "DECATHLON",
    "245": "unused",
    "246": "FONG SHEN BANG",
    "247": "unused",
    "248": "unused",
    "249": "unused",
    "250": "unused",
    "251": "unused",
    "252": "SAN GUO ZHI PIRATE",
    "253": "DRAGON BALL PIRATE",
    "254": "unused",
    "255": "unused"
}

def get_mapper(mapper=0, path='./'):
    """Scrape individual mapper"""

    output = []

    url = 'http://wiki.nesdev.com/w/index.php/INES_Mapper_%03d' % (mapper)
    response = requests.get(url)
    html = BeautifulSoup(response.text, 'html.parser')

    title = mapper_list['%03d' % (mapper)]

    info = html.find('div', {
        'class': 'infobox'
    })

    if info:
        info_rows = info.findAll('tr')
        subjects = []
        contents = []

        try:
            for info_row in info_rows:
                info_cols = info_row.findAll('td')

                subject = ''.join(info_cols[0].findAll(text=True, recursive=True)).strip()
                content = ''.join(info_cols[1].findAll(text=True, recursive=True)).strip()

                content = re.sub(u'\xd7', 'x', content)

                subjects.append(subject)
                contents.append(content)

            subject_length = 0

            for i in range(0, len(subjects)):
                subject_length = max(subject_length, len(subjects[i]))

            subject_length -= 1

            output.append('%s (Mapper %03d)\n' % (title, mapper))

            for i in range(0, len(subjects)):
                length = subject_length - len(subjects[i])

                if length > 0:
                    output.append('%s:%s  %s' % (subjects[i], ' ' * length, contents[i]))
                else:
                    output.append('%s: %s' % (subjects[i], contents[i]))
        except IndexError:
            output.append('%s (Mapper %03d)\n' % (title, mapper))
            return 1
    else:
        output.append('%s (Mapper %03d)\n' % (title, mapper))

    slug = re.sub(r'_+', '_', re.sub(r'[^a-zA-Z0-9]', '_', mapper_list['%03d' % (mapper)].lower()))

    if slug == 'unused':
        slug += '_%03d' % (mapper)

    filename = '%s%s.txt' % (path, slug)

    with open(filename, 'w') as f:
        f.write('\n'.join(output))

    print '{ "%d", reference_mappers_%s_txt, &reference_mappers_%s_txt_len },' % (mapper, slug, slug)

    return 0

def main():
    """Main function"""

    parser = argparse.ArgumentParser(description='Mapper scraper')

    parser.add_argument('--mapper', dest='mapper', type=int, required=False, default=-1, help='Mapper to scrape')
    parser.add_argument('--all', dest='all', action='store_true', required=False, default=False, help='Scrape all')
    parser.add_argument('--output', dest='output', type=str, required=False, default='./', help='Output directory')

    args = parser.parse_args()

    if not args.all and args.mapper == -1:
        parser.print_help()
        sys.exit(1)

    if args.all:
        for i in range(0, 256):
            get_mapper(i, args.output)
    else:
        if args.mapper == -1:
            parser.print_help()
        else:
            get_mapper(args.mapper, args.output)

if __name__ == '__main__':
    main()
