#ifndef _REFERENCE_H
#define _REFERENCE_H

#include "reference/ppuctrl.h"
#include "reference/ppumask.h"
#include "reference/ppustatus.h"
#include "reference/oamaddr.h"
#include "reference/oamdata.h"
#include "reference/ppuscroll.h"
#include "reference/ppuaddr.h"
#include "reference/ppudata.h"

#define MAX_TERMS      10
#define MAX_CATEGORIES 10

struct term {
    char *name;
    unsigned char *description;
    unsigned int *len;
};

struct category {
    char *name;
    struct term terms[MAX_TERMS];
};

struct category categories[MAX_CATEGORIES] = {
    {
        "registers", {
            { "PPUCTRL",   reference_ppuctrl_txt,   &reference_ppuctrl_txt_len },
            { "PPUMASK",   reference_ppumask_txt,   &reference_ppumask_txt_len },
            { "PPUSTATUS", reference_ppustatus_txt, &reference_ppustatus_txt_len },
            { "OAMADDR",   reference_oamaddr_txt,   &reference_oamaddr_txt_len },
            { "OAMDATA",   reference_oamdata_txt,   &reference_oamdata_txt_len },
            { "PPUSCROLL", reference_ppuscroll_txt, &reference_ppuscroll_txt_len },
            { "PPUADDR",   reference_ppuaddr_txt,   &reference_ppuaddr_txt_len },
            { "PPUDATA",   reference_ppudata_txt,   &reference_ppudata_txt_len }
        }
    }
};

#endif /* _REFERENCE_H */
