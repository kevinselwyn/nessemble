#ifndef _REFERENCE_H
#define _REFERENCE_H

#include "reference/registers/ppuctrl.h"
#include "reference/registers/ppumask.h"
#include "reference/registers/ppustatus.h"
#include "reference/registers/oamaddr.h"
#include "reference/registers/oamdata.h"
#include "reference/registers/ppuscroll.h"
#include "reference/registers/ppuaddr.h"
#include "reference/registers/ppudata.h"

#include "reference/addressing/accumulator.h"
#include "reference/addressing/implied.h"
#include "reference/addressing/immediate.h"
#include "reference/addressing/relative.h"
#include "reference/addressing/zeropage.h"
#include "reference/addressing/zeropage-x.h"
#include "reference/addressing/zeropage-y.h"
#include "reference/addressing/absolute.h"
#include "reference/addressing/absolute-x.h"
#include "reference/addressing/absolute-y.h"
#include "reference/addressing/indirect.h"
#include "reference/addressing/indirect-x.h"
#include "reference/addressing/indirect-y.h"

#define MAX_TERMS      13
#define MAX_CATEGORIES 2

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
            { "PPUCTRL",   reference_registers_ppuctrl_txt,   &reference_registers_ppuctrl_txt_len },
            { "PPUMASK",   reference_registers_ppumask_txt,   &reference_registers_ppumask_txt_len },
            { "PPUSTATUS", reference_registers_ppustatus_txt, &reference_registers_ppustatus_txt_len },
            { "OAMADDR",   reference_registers_oamaddr_txt,   &reference_registers_oamaddr_txt_len },
            { "OAMDATA",   reference_registers_oamdata_txt,   &reference_registers_oamdata_txt_len },
            { "PPUSCROLL", reference_registers_ppuscroll_txt, &reference_registers_ppuscroll_txt_len },
            { "PPUADDR",   reference_registers_ppuaddr_txt,   &reference_registers_ppuaddr_txt_len },
            { "PPUDATA",   reference_registers_ppudata_txt,   &reference_registers_ppudata_txt_len }
        }
    },
    {
        "addressing", {
            { "accumulator", reference_addressing_accumulator_txt, &reference_addressing_accumulator_txt_len },
            { "implied",     reference_addressing_implied_txt,     &reference_addressing_implied_txt_len },
            { "immediate",   reference_addressing_immediate_txt,   &reference_addressing_immediate_txt_len },
            { "relative",    reference_addressing_relative_txt,    &reference_addressing_relative_txt_len },
            { "zeropage",    reference_addressing_zeropage_txt,    &reference_addressing_zeropage_txt_len },
            { "zeropage-x",  reference_addressing_zeropage_x_txt,  &reference_addressing_zeropage_x_txt_len },
            { "zeropage-y",  reference_addressing_zeropage_y_txt,  &reference_addressing_zeropage_y_txt_len },
            { "absolute",    reference_addressing_absolute_txt,    &reference_addressing_absolute_txt_len },
            { "absolute-x",  reference_addressing_absolute_x_txt,  &reference_addressing_absolute_x_txt_len },
            { "absolute-y",  reference_addressing_absolute_y_txt,  &reference_addressing_absolute_y_txt_len },
            { "indirect",    reference_addressing_indirect_txt,    &reference_addressing_indirect_txt_len },
            { "indirect-x",  reference_addressing_indirect_x_txt,  &reference_addressing_indirect_x_txt_len },
            { "indirect-y",  reference_addressing_indirect_y_txt,  &reference_addressing_indirect_y_txt_len }
        }
    }
};

#endif /* _REFERENCE_H */
