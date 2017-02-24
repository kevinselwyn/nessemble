#ifndef _REFERENCE_H
#define _REFERENCE_H

#define MAX_CATEGORIES 3
#define MAX_TERMS      256

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

#include "reference/mappers/nrom.h"
#include "reference/mappers/mmc1.h"
#include "reference/mappers/unrom.h"
#include "reference/mappers/cnrom.h"
#include "reference/mappers/mmc3.h"
#include "reference/mappers/mmc5.h"
#include "reference/mappers/ffe_rev_a.h"
#include "reference/mappers/anrom.h"
#include "reference/mappers/mmc2.h"
#include "reference/mappers/mmc4.h"
#include "reference/mappers/color_dreams.h"
#include "reference/mappers/rex_dbz_5.h"
#include "reference/mappers/cprom.h"
#include "reference/mappers/rex_sl_1632.h"
#include "reference/mappers/100_in_1.h"
#include "reference/mappers/bandai_24c02.h"
#include "reference/mappers/ffe_rev_b.h"
#include "reference/mappers/jaleco_ss880006.h"
#include "reference/mappers/namcot_106.h"
#include "reference/mappers/konami_vrc2_vrc4_a.h"
#include "reference/mappers/konami_vrc2_vrc4_b.h"
#include "reference/mappers/konami_vrc2_vrc4_c.h"
#include "reference/mappers/konami_vrc6_rev_a.h"
#include "reference/mappers/konami_vrc2_vrc4_d.h"
#include "reference/mappers/konami_vrc6_rev_b.h"
#include "reference/mappers/cc_21_mi_hun_che.h"
#include "reference/mappers/ret_cufrom.h"
#include "reference/mappers/unrom_512.h"
#include "reference/mappers/infiniteneslives_nsf.h"
#include "reference/mappers/irem_g_101.h"
#include "reference/mappers/tc0190fmc_tc0350fmr.h"
#include "reference/mappers/irem_i_im_bnrom.h"
#include "reference/mappers/wario_land_2.h"
#include "reference/mappers/txc_policeman.h"
#include "reference/mappers/pal_zz_smb_tetris_nwc.h"
#include "reference/mappers/bit_corp_.h"
#include "reference/mappers/smb2j_fds.h"
#include "reference/mappers/caltron_6_in_1.h"
#include "reference/mappers/bio_miracle_fds.h"
#include "reference/mappers/fds_smb2j_lf36.h"
#include "reference/mappers/mmc3_bmc_pirate_a.h"
#include "reference/mappers/mmc3_bmc_pirate_b.h"
#include "reference/mappers/rumblestation_15_in_1.h"
#include "reference/mappers/nes_qj_ssvb_nwc.h"
#include "reference/mappers/taito_tcxxx.h"
#include "reference/mappers/mmc3_bmc_pirate_c.h"
#include "reference/mappers/smb2j_fds_rev_a.h"
#include "reference/mappers/11_in_1_ball_series.h"
#include "reference/mappers/mmc3_bmc_pirate_d.h"
#include "reference/mappers/supervision_16_in_1.h"
#include "reference/mappers/simbple_bmc_pirate_a.h"
#include "reference/mappers/simbple_bmc_pirate_b.h"
#include "reference/mappers/simbple_bmc_pirate_c.h"
#include "reference/mappers/20_in_1_kaiser_rev_a.h"
#include "reference/mappers/700_in_1.h"
#include "reference/mappers/tengen_rambo1.h"
#include "reference/mappers/irem_h3001.h"
#include "reference/mappers/mhrom.h"
#include "reference/mappers/sunsoft_fzii.h"
#include "reference/mappers/sunsoft_mapper_4.h"
#include "reference/mappers/sunsoft_5_fme_7.h"
#include "reference/mappers/ba_kamen_discrete.h"
#include "reference/mappers/camerica_bf9093.h"
#include "reference/mappers/jaleco_jf_17.h"
#include "reference/mappers/konami_vrc3.h"
#include "reference/mappers/tw_mmc3_vram_rev_a.h"
#include "reference/mappers/konami_vrc1.h"
#include "reference/mappers/namcot_108_rev_a.h"
#include "reference/mappers/irem_lrog017.h"
#include "reference/mappers/irem_74hc161_32.h"
#include "reference/mappers/ave_c_e_txc_board.h"
#include "reference/mappers/taito_x1_005_rev_a.h"
#include "reference/mappers/taito_x1_017.h"
#include "reference/mappers/yoko_vrc_rev_b.h"
#include "reference/mappers/konami_vrc7.h"
#include "reference/mappers/jaleco_jf_13.h"
#include "reference/mappers/74_139_74_discrete.h"
#include "reference/mappers/namco_3433.h"
#include "reference/mappers/sunsoft_3.h"
#include "reference/mappers/hummer_jy_board.h"
#include "reference/mappers/early_hummer_jy_board.h"
#include "reference/mappers/jaleco_jf_19.h"
#include "reference/mappers/sunsoft_3r.h"
#include "reference/mappers/namcot_108_rev_b.h"
#include "reference/mappers/bandai_oekakids.h"
#include "reference/mappers/irem_tam_s1.h"
#include "reference/mappers/vs_uni_dual_system.h"
#include "reference/mappers/fds_dokidoki_full.h"
#include "reference/mappers/nes_event_nwc1990.h"
#include "reference/mappers/smb3_pirate_a.h"
#include "reference/mappers/magic_corp_a.h"
#include "reference/mappers/fds_unrom_board.h"
#include "reference/mappers/asder_ntdec_board.h"
#include "reference/mappers/hacker_sachen_board.h"
#include "reference/mappers/mmc3_sg_prot_a.h"
#include "reference/mappers/mmc3_pirate_a.h"
#include "reference/mappers/mmc1_mmc3_vrc_pirate.h"
#include "reference/mappers/future_media_board.h"
#include "reference/mappers/tskrom.h"
#include "reference/mappers/nes_tqrom.h"
#include "reference/mappers/fds_tobidase.h"
#include "reference/mappers/mmc3_pirate_prot_a.h"
#include "reference/mappers/mmc3_pirate_h2288.h"
#include "reference/mappers/fds_lh32.h"
#include "reference/mappers/txc_mgenius_22111.h"
#include "reference/mappers/sa72008.h"
#include "reference/mappers/mmc3_bmc_pirate.h"
#include "reference/mappers/tcu02.h"
#include "reference/mappers/s8259d.h"
#include "reference/mappers/s8259b.h"
#include "reference/mappers/s8259c.h"
#include "reference/mappers/jaleco_jf_11_14.h"
#include "reference/mappers/s8259a.h"
#include "reference/mappers/unlks7032.h"
#include "reference/mappers/tca01.h"
#include "reference/mappers/agci_50282.h"
#include "reference/mappers/sa72007.h"
#include "reference/mappers/sa0161m.h"
#include "reference/mappers/tcu01.h"
#include "reference/mappers/sa0037.h"
#include "reference/mappers/sa0036.h"
#include "reference/mappers/s74ls374n.h"
#include "reference/mappers/bandai_sram.h"
#include "reference/mappers/bandai_barcode.h"
#include "reference/mappers/bandai_24c01.h"
#include "reference/mappers/sa009.h"
#include "reference/mappers/subor_rev_a.h"
#include "reference/mappers/subor_rev_b.h"
#include "reference/mappers/bmcfk23c.h"
#include "reference/mappers/tw_mmc3_vram_rev_b.h"
#include "reference/mappers/ntdec_tc_112.h"
#include "reference/mappers/tw_mmc3_vram_rev_c.h"
#include "reference/mappers/tw_mmc3_vram_rev_d.h"
#include "reference/mappers/tw_mmc3_vram_rev_e.h"
#include "reference/mappers/namcot_108_rev_c.h"
#include "reference/mappers/taito_x1_005_rev_b.h"
#include "reference/mappers/unla9746.h"
#include "reference/mappers/debug_mapper.h"
#include "reference/mappers/unln625092.h"
#include "reference/mappers/bmc_22_20_in_1.h"
#include "reference/mappers/bmc_contra_22_in_1.h"
#include "reference/mappers/bmc_quattro.h"
#include "reference/mappers/bmc_22_20_in_1_rst.h"
#include "reference/mappers/bmc_maxi.h"
#include "reference/mappers/unl6035052.h"
#include "reference/mappers/s74ls374na.h"
#include "reference/mappers/decathlon.h"
#include "reference/mappers/fong_shen_bang.h"
#include "reference/mappers/san_guo_zhi_pirate.h"
#include "reference/mappers/dragon_ball_pirate.h"
#include "reference/mappers/unused.h"

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
            { "PPUCTRL",   reference_registers_ppuctrl_txt,   &reference_registers_ppuctrl_txt_len   },
            { "PPUMASK",   reference_registers_ppumask_txt,   &reference_registers_ppumask_txt_len   },
            { "PPUSTATUS", reference_registers_ppustatus_txt, &reference_registers_ppustatus_txt_len },
            { "OAMADDR",   reference_registers_oamaddr_txt,   &reference_registers_oamaddr_txt_len   },
            { "OAMDATA",   reference_registers_oamdata_txt,   &reference_registers_oamdata_txt_len   },
            { "PPUSCROLL", reference_registers_ppuscroll_txt, &reference_registers_ppuscroll_txt_len },
            { "PPUADDR",   reference_registers_ppuaddr_txt,   &reference_registers_ppuaddr_txt_len   },
            { "PPUDATA",   reference_registers_ppudata_txt,   &reference_registers_ppudata_txt_len   }
        }
    },
    {
        "addressing", {
            { "accumulator", reference_addressing_accumulator_txt, &reference_addressing_accumulator_txt_len },
            { "implied",     reference_addressing_implied_txt,     &reference_addressing_implied_txt_len     },
            { "immediate",   reference_addressing_immediate_txt,   &reference_addressing_immediate_txt_len   },
            { "relative",    reference_addressing_relative_txt,    &reference_addressing_relative_txt_len    },
            { "zeropage",    reference_addressing_zeropage_txt,    &reference_addressing_zeropage_txt_len    },
            { "zeropage-x",  reference_addressing_zeropage_x_txt,  &reference_addressing_zeropage_x_txt_len  },
            { "zeropage-y",  reference_addressing_zeropage_y_txt,  &reference_addressing_zeropage_y_txt_len  },
            { "absolute",    reference_addressing_absolute_txt,    &reference_addressing_absolute_txt_len    },
            { "absolute-x",  reference_addressing_absolute_x_txt,  &reference_addressing_absolute_x_txt_len  },
            { "absolute-y",  reference_addressing_absolute_y_txt,  &reference_addressing_absolute_y_txt_len  },
            { "indirect",    reference_addressing_indirect_txt,    &reference_addressing_indirect_txt_len    },
            { "indirect-x",  reference_addressing_indirect_x_txt,  &reference_addressing_indirect_x_txt_len  },
            { "indirect-y",  reference_addressing_indirect_y_txt,  &reference_addressing_indirect_y_txt_len  }
        }
    },
    {
        "mappers", {
            { "0",   reference_mappers_nrom_txt, &reference_mappers_nrom_txt_len },
            { "1",   reference_mappers_mmc1_txt, &reference_mappers_mmc1_txt_len },
            { "2",   reference_mappers_unrom_txt, &reference_mappers_unrom_txt_len },
            { "3",   reference_mappers_cnrom_txt, &reference_mappers_cnrom_txt_len },
            { "4",   reference_mappers_mmc3_txt, &reference_mappers_mmc3_txt_len },
            { "5",   reference_mappers_mmc5_txt, &reference_mappers_mmc5_txt_len },
            { "6",   reference_mappers_ffe_rev_a_txt, &reference_mappers_ffe_rev_a_txt_len },
            { "7",   reference_mappers_anrom_txt, &reference_mappers_anrom_txt_len },
            { "8",   reference_mappers_unused_txt, &reference_mappers_unused_txt_len },
            { "9",   reference_mappers_mmc2_txt, &reference_mappers_mmc2_txt_len },
            { "10",  reference_mappers_mmc4_txt, &reference_mappers_mmc4_txt_len },
            { "11",  reference_mappers_color_dreams_txt, &reference_mappers_color_dreams_txt_len },
            { "12",  reference_mappers_rex_dbz_5_txt, &reference_mappers_rex_dbz_5_txt_len },
            { "13",  reference_mappers_cprom_txt, &reference_mappers_cprom_txt_len },
            { "14",  reference_mappers_rex_sl_1632_txt, &reference_mappers_rex_sl_1632_txt_len },
            { "15",  reference_mappers_100_in_1_txt, &reference_mappers_100_in_1_txt_len },
            { "16",  reference_mappers_bandai_24c02_txt, &reference_mappers_bandai_24c02_txt_len },
            { "17",  reference_mappers_ffe_rev_b_txt, &reference_mappers_ffe_rev_b_txt_len },
            { "18",  reference_mappers_jaleco_ss880006_txt, &reference_mappers_jaleco_ss880006_txt_len },
            { "19",  reference_mappers_namcot_106_txt, &reference_mappers_namcot_106_txt_len },
            { "20",  reference_mappers_unused_txt, &reference_mappers_unused_txt_len },
            { "21",  reference_mappers_konami_vrc2_vrc4_a_txt, &reference_mappers_konami_vrc2_vrc4_a_txt_len },
            { "22",  reference_mappers_konami_vrc2_vrc4_b_txt, &reference_mappers_konami_vrc2_vrc4_b_txt_len },
            { "23",  reference_mappers_konami_vrc2_vrc4_c_txt, &reference_mappers_konami_vrc2_vrc4_c_txt_len },
            { "24",  reference_mappers_konami_vrc6_rev_a_txt, &reference_mappers_konami_vrc6_rev_a_txt_len },
            { "25",  reference_mappers_konami_vrc2_vrc4_d_txt, &reference_mappers_konami_vrc2_vrc4_d_txt_len },
            { "26",  reference_mappers_konami_vrc6_rev_b_txt, &reference_mappers_konami_vrc6_rev_b_txt_len },
            { "27",  reference_mappers_cc_21_mi_hun_che_txt, &reference_mappers_cc_21_mi_hun_che_txt_len },
            { "28",  reference_mappers_unused_txt, &reference_mappers_unused_txt_len },
            { "29",  reference_mappers_ret_cufrom_txt, &reference_mappers_ret_cufrom_txt_len },
            { "30",  reference_mappers_unrom_512_txt, &reference_mappers_unrom_512_txt_len },
            { "31",  reference_mappers_infiniteneslives_nsf_txt, &reference_mappers_infiniteneslives_nsf_txt_len },
            { "32",  reference_mappers_irem_g_101_txt, &reference_mappers_irem_g_101_txt_len },
            { "33",  reference_mappers_tc0190fmc_tc0350fmr_txt, &reference_mappers_tc0190fmc_tc0350fmr_txt_len },
            { "34",  reference_mappers_irem_i_im_bnrom_txt, &reference_mappers_irem_i_im_bnrom_txt_len },
            { "35",  reference_mappers_wario_land_2_txt, &reference_mappers_wario_land_2_txt_len },
            { "36",  reference_mappers_txc_policeman_txt, &reference_mappers_txc_policeman_txt_len },
            { "37",  reference_mappers_pal_zz_smb_tetris_nwc_txt, &reference_mappers_pal_zz_smb_tetris_nwc_txt_len },
            { "38",  reference_mappers_bit_corp__txt, &reference_mappers_bit_corp__txt_len },
            { "39",  reference_mappers_unused_txt, &reference_mappers_unused_txt_len },
            { "40",  reference_mappers_smb2j_fds_txt, &reference_mappers_smb2j_fds_txt_len },
            { "41",  reference_mappers_caltron_6_in_1_txt, &reference_mappers_caltron_6_in_1_txt_len },
            { "42",  reference_mappers_bio_miracle_fds_txt, &reference_mappers_bio_miracle_fds_txt_len },
            { "43",  reference_mappers_fds_smb2j_lf36_txt, &reference_mappers_fds_smb2j_lf36_txt_len },
            { "44",  reference_mappers_mmc3_bmc_pirate_a_txt, &reference_mappers_mmc3_bmc_pirate_a_txt_len },
            { "45",  reference_mappers_mmc3_bmc_pirate_b_txt, &reference_mappers_mmc3_bmc_pirate_b_txt_len },
            { "46",  reference_mappers_rumblestation_15_in_1_txt, &reference_mappers_rumblestation_15_in_1_txt_len },
            { "47",  reference_mappers_nes_qj_ssvb_nwc_txt, &reference_mappers_nes_qj_ssvb_nwc_txt_len },
            { "48",  reference_mappers_taito_tcxxx_txt, &reference_mappers_taito_tcxxx_txt_len },
            { "49",  reference_mappers_mmc3_bmc_pirate_c_txt, &reference_mappers_mmc3_bmc_pirate_c_txt_len },
            { "50",  reference_mappers_smb2j_fds_rev_a_txt, &reference_mappers_smb2j_fds_rev_a_txt_len },
            { "51",  reference_mappers_11_in_1_ball_series_txt, &reference_mappers_11_in_1_ball_series_txt_len },
            { "52",  reference_mappers_mmc3_bmc_pirate_d_txt, &reference_mappers_mmc3_bmc_pirate_d_txt_len },
            { "53",  reference_mappers_supervision_16_in_1_txt, &reference_mappers_supervision_16_in_1_txt_len },
            { "54",  reference_mappers_unused_txt, &reference_mappers_unused_txt_len },
            { "55",  reference_mappers_unused_txt, &reference_mappers_unused_txt_len },
            { "56",  reference_mappers_unused_txt, &reference_mappers_unused_txt_len },
            { "57",  reference_mappers_simbple_bmc_pirate_a_txt, &reference_mappers_simbple_bmc_pirate_a_txt_len },
            { "58",  reference_mappers_simbple_bmc_pirate_b_txt, &reference_mappers_simbple_bmc_pirate_b_txt_len },
            { "59",  reference_mappers_unused_txt, &reference_mappers_unused_txt_len },
            { "60",  reference_mappers_simbple_bmc_pirate_c_txt, &reference_mappers_simbple_bmc_pirate_c_txt_len },
            { "61",  reference_mappers_20_in_1_kaiser_rev_a_txt, &reference_mappers_20_in_1_kaiser_rev_a_txt_len },
            { "62",  reference_mappers_700_in_1_txt, &reference_mappers_700_in_1_txt_len },
            { "63",  reference_mappers_unused_txt, &reference_mappers_unused_txt_len },
            { "64",  reference_mappers_tengen_rambo1_txt, &reference_mappers_tengen_rambo1_txt_len },
            { "65",  reference_mappers_irem_h3001_txt, &reference_mappers_irem_h3001_txt_len },
            { "66",  reference_mappers_mhrom_txt, &reference_mappers_mhrom_txt_len },
            { "67",  reference_mappers_sunsoft_fzii_txt, &reference_mappers_sunsoft_fzii_txt_len },
            { "68",  reference_mappers_sunsoft_mapper_4_txt, &reference_mappers_sunsoft_mapper_4_txt_len },
            { "69",  reference_mappers_sunsoft_5_fme_7_txt, &reference_mappers_sunsoft_5_fme_7_txt_len },
            { "70",  reference_mappers_ba_kamen_discrete_txt, &reference_mappers_ba_kamen_discrete_txt_len },
            { "71",  reference_mappers_camerica_bf9093_txt, &reference_mappers_camerica_bf9093_txt_len },
            { "72",  reference_mappers_jaleco_jf_17_txt, &reference_mappers_jaleco_jf_17_txt_len },
            { "73",  reference_mappers_konami_vrc3_txt, &reference_mappers_konami_vrc3_txt_len },
            { "74",  reference_mappers_tw_mmc3_vram_rev_a_txt, &reference_mappers_tw_mmc3_vram_rev_a_txt_len },
            { "75",  reference_mappers_konami_vrc1_txt, &reference_mappers_konami_vrc1_txt_len },
            { "76",  reference_mappers_namcot_108_rev_a_txt, &reference_mappers_namcot_108_rev_a_txt_len },
            { "77",  reference_mappers_irem_lrog017_txt, &reference_mappers_irem_lrog017_txt_len },
            { "78",  reference_mappers_irem_74hc161_32_txt, &reference_mappers_irem_74hc161_32_txt_len },
            { "79",  reference_mappers_ave_c_e_txc_board_txt, &reference_mappers_ave_c_e_txc_board_txt_len },
            { "80",  reference_mappers_taito_x1_005_rev_a_txt, &reference_mappers_taito_x1_005_rev_a_txt_len },
            { "81",  reference_mappers_unused_txt, &reference_mappers_unused_txt_len },
            { "82",  reference_mappers_taito_x1_017_txt, &reference_mappers_taito_x1_017_txt_len },
            { "83",  reference_mappers_yoko_vrc_rev_b_txt, &reference_mappers_yoko_vrc_rev_b_txt_len },
            { "84",  reference_mappers_unused_txt, &reference_mappers_unused_txt_len },
            { "85",  reference_mappers_konami_vrc7_txt, &reference_mappers_konami_vrc7_txt_len },
            { "86",  reference_mappers_jaleco_jf_13_txt, &reference_mappers_jaleco_jf_13_txt_len },
            { "87",  reference_mappers_74_139_74_discrete_txt, &reference_mappers_74_139_74_discrete_txt_len },
            { "88",  reference_mappers_namco_3433_txt, &reference_mappers_namco_3433_txt_len },
            { "89",  reference_mappers_sunsoft_3_txt, &reference_mappers_sunsoft_3_txt_len },
            { "90",  reference_mappers_hummer_jy_board_txt, &reference_mappers_hummer_jy_board_txt_len },
            { "91",  reference_mappers_early_hummer_jy_board_txt, &reference_mappers_early_hummer_jy_board_txt_len },
            { "92",  reference_mappers_jaleco_jf_19_txt,          &reference_mappers_jaleco_jf_19_txt_len          },
            { "93",  reference_mappers_sunsoft_3r_txt,            &reference_mappers_sunsoft_3r_txt_len            },
            { "95",  reference_mappers_namcot_108_rev_b_txt,      &reference_mappers_namcot_108_rev_b_txt_len      },
            { "96",  reference_mappers_bandai_oekakids_txt,       &reference_mappers_bandai_oekakids_txt_len       },
            { "97",  reference_mappers_irem_tam_s1_txt,           &reference_mappers_irem_tam_s1_txt_len           },
            { "98",  reference_mappers_unused_txt,                &reference_mappers_unused_txt_len                },
            { "99",  reference_mappers_vs_uni_dual_system_txt,    &reference_mappers_vs_uni_dual_system_txt_len    },
            { "100", reference_mappers_unused_txt,                &reference_mappers_unused_txt_len                },
            { "101", reference_mappers_unused_txt,                &reference_mappers_unused_txt_len                },
            { "102", reference_mappers_unused_txt,                &reference_mappers_unused_txt_len                },
            { "103", reference_mappers_fds_dokidoki_full_txt,     &reference_mappers_fds_dokidoki_full_txt_len     },
            { "104", reference_mappers_unused_txt,                &reference_mappers_unused_txt_len                },
            { "105", reference_mappers_nes_event_nwc1990_txt,     &reference_mappers_nes_event_nwc1990_txt_len     },
            { "106", reference_mappers_smb3_pirate_a_txt,         &reference_mappers_smb3_pirate_a_txt_len         },
            { "107", reference_mappers_magic_corp_a_txt,          &reference_mappers_magic_corp_a_txt_len          },
            { "108", reference_mappers_fds_unrom_board_txt,       &reference_mappers_fds_unrom_board_txt_len       },
            { "109", reference_mappers_unused_txt,                &reference_mappers_unused_txt_len                },
            { "110", reference_mappers_unused_txt,                &reference_mappers_unused_txt_len                },
            { "111", reference_mappers_unused_txt,                &reference_mappers_unused_txt_len                },
            { "112", reference_mappers_asder_ntdec_board_txt,     &reference_mappers_asder_ntdec_board_txt_len     },
            { "113", reference_mappers_hacker_sachen_board_txt,   &reference_mappers_hacker_sachen_board_txt_len   },
            { "114", reference_mappers_mmc3_sg_prot_a_txt,        &reference_mappers_mmc3_sg_prot_a_txt_len        },
            { "115", reference_mappers_mmc3_pirate_a_txt,         &reference_mappers_mmc3_pirate_a_txt_len         },
            { "116", reference_mappers_mmc1_mmc3_vrc_pirate_txt,  &reference_mappers_mmc1_mmc3_vrc_pirate_txt_len  },
            { "117", reference_mappers_future_media_board_txt,    &reference_mappers_future_media_board_txt_len    },
            { "118", reference_mappers_tskrom_txt,                &reference_mappers_tskrom_txt_len                },
            { "119", reference_mappers_nes_tqrom_txt,             &reference_mappers_nes_tqrom_txt_len             },
            { "120", reference_mappers_fds_tobidase_txt,          &reference_mappers_fds_tobidase_txt_len          },
            { "121", reference_mappers_mmc3_pirate_prot_a_txt,    &reference_mappers_mmc3_pirate_prot_a_txt_len    },
            { "122", reference_mappers_unused_txt,                &reference_mappers_unused_txt_len                },
            { "123", reference_mappers_mmc3_pirate_h2288_txt,     &reference_mappers_mmc3_pirate_h2288_txt_len     },
            { "124", reference_mappers_unused_txt,                &reference_mappers_unused_txt_len                },
            { "125", reference_mappers_fds_lh32_txt,              &reference_mappers_fds_lh32_txt_len              },
            { "126", reference_mappers_unused_txt,                &reference_mappers_unused_txt_len                },
            { "127", reference_mappers_unused_txt,                &reference_mappers_unused_txt_len                },
            { "128", reference_mappers_unused_txt,                &reference_mappers_unused_txt_len                },
            { "129", reference_mappers_unused_txt,                &reference_mappers_unused_txt_len                },
            { "130", reference_mappers_unused_txt,                &reference_mappers_unused_txt_len                },
            { "131", reference_mappers_unused_txt,                &reference_mappers_unused_txt_len                },
            { "132", reference_mappers_txc_mgenius_22111_txt,     &reference_mappers_txc_mgenius_22111_txt_len     },
            { "133", reference_mappers_sa72008_txt,               &reference_mappers_sa72008_txt_len               },
            { "134", reference_mappers_mmc3_bmc_pirate_txt,       &reference_mappers_mmc3_bmc_pirate_txt_len       },
            { "135", reference_mappers_unused_txt,                &reference_mappers_unused_txt_len                },
            { "136", reference_mappers_tcu02_txt,                 &reference_mappers_tcu02_txt_len                 },
            { "137", reference_mappers_s8259d_txt,                &reference_mappers_s8259d_txt_len                },
            { "138", reference_mappers_s8259b_txt,                &reference_mappers_s8259b_txt_len                },
            { "139", reference_mappers_s8259c_txt,                &reference_mappers_s8259c_txt_len                },
            { "140", reference_mappers_jaleco_jf_11_14_txt,       &reference_mappers_jaleco_jf_11_14_txt_len       },
            { "141", reference_mappers_s8259a_txt,                &reference_mappers_s8259a_txt_len                },
            { "142", reference_mappers_unlks7032_txt,             &reference_mappers_unlks7032_txt_len             },
            { "143", reference_mappers_tca01_txt,                 &reference_mappers_tca01_txt_len                 },
            { "144", reference_mappers_agci_50282_txt,            &reference_mappers_agci_50282_txt_len            },
            { "145", reference_mappers_sa72007_txt,               &reference_mappers_sa72007_txt_len               },
            { "146", reference_mappers_sa0161m_txt,               &reference_mappers_sa0161m_txt_len               },
            { "147", reference_mappers_tcu01_txt,                 &reference_mappers_tcu01_txt_len                 },
            { "148", reference_mappers_sa0037_txt,                &reference_mappers_sa0037_txt_len                },
            { "149", reference_mappers_sa0036_txt,                &reference_mappers_sa0036_txt_len                },
            { "150", reference_mappers_s74ls374n_txt,             &reference_mappers_s74ls374n_txt_len             },
            { "151", reference_mappers_unused_txt,                &reference_mappers_unused_txt_len                },
            { "152", reference_mappers_unused_txt,                &reference_mappers_unused_txt_len                },
            { "153", reference_mappers_bandai_sram_txt,           &reference_mappers_bandai_sram_txt_len           },
            { "154", reference_mappers_unused_txt,                &reference_mappers_unused_txt_len                },
            { "155", reference_mappers_unused_txt,                &reference_mappers_unused_txt_len                },
            { "156", reference_mappers_unused_txt,                &reference_mappers_unused_txt_len                },
            { "157", reference_mappers_bandai_barcode_txt,        &reference_mappers_bandai_barcode_txt_len        },
            { "158", reference_mappers_unused_txt,                &reference_mappers_unused_txt_len                },
            { "159", reference_mappers_bandai_24c01_txt,          &reference_mappers_bandai_24c01_txt_len          },
            { "160", reference_mappers_sa009_txt,                 &reference_mappers_sa009_txt_len                 },
            { "161", reference_mappers_unused_txt,                &reference_mappers_unused_txt_len                },
            { "162", reference_mappers_unused_txt,                &reference_mappers_unused_txt_len                },
            { "163", reference_mappers_unused_txt,                &reference_mappers_unused_txt_len                },
            { "164", reference_mappers_unused_txt,                &reference_mappers_unused_txt_len                },
            { "165", reference_mappers_unused_txt,                &reference_mappers_unused_txt_len                },
            { "166", reference_mappers_subor_rev_a_txt,           &reference_mappers_subor_rev_a_txt_len           },
            { "167", reference_mappers_subor_rev_b_txt,           &reference_mappers_subor_rev_b_txt_len           },
            { "168", reference_mappers_unused_txt,                &reference_mappers_unused_txt_len                },
            { "169", reference_mappers_unused_txt,                &reference_mappers_unused_txt_len                },
            { "170", reference_mappers_unused_txt,                &reference_mappers_unused_txt_len                },
            { "171", reference_mappers_unused_txt,                &reference_mappers_unused_txt_len                },
            { "172", reference_mappers_unused_txt,                &reference_mappers_unused_txt_len                },
            { "173", reference_mappers_unused_txt,                &reference_mappers_unused_txt_len                },
            { "174", reference_mappers_unused_txt,                &reference_mappers_unused_txt_len                },
            { "175", reference_mappers_unused_txt,                &reference_mappers_unused_txt_len                },
            { "176", reference_mappers_bmcfk23c_txt,              &reference_mappers_bmcfk23c_txt_len              },
            { "177", reference_mappers_unused_txt,                &reference_mappers_unused_txt_len                },
            { "178", reference_mappers_unused_txt,                &reference_mappers_unused_txt_len                },
            { "179", reference_mappers_unused_txt,                &reference_mappers_unused_txt_len                },
            { "181", reference_mappers_unused_txt,                &reference_mappers_unused_txt_len                },
            { "182", reference_mappers_unused_txt,                &reference_mappers_unused_txt_len                },
            { "183", reference_mappers_unused_txt,                &reference_mappers_unused_txt_len                },
            { "184", reference_mappers_unused_txt,                &reference_mappers_unused_txt_len                },
            { "185", reference_mappers_unused_txt,                &reference_mappers_unused_txt_len                },
            { "186", reference_mappers_unused_txt,                &reference_mappers_unused_txt_len                },
            { "187", reference_mappers_unused_txt,                &reference_mappers_unused_txt_len                },
            { "188", reference_mappers_unused_txt,                &reference_mappers_unused_txt_len                },
            { "189", reference_mappers_unused_txt,                &reference_mappers_unused_txt_len                },
            { "190", reference_mappers_unused_txt,                &reference_mappers_unused_txt_len                },
            { "191", reference_mappers_unused_txt,                &reference_mappers_unused_txt_len                },
            { "192", reference_mappers_tw_mmc3_vram_rev_b_txt,    &reference_mappers_tw_mmc3_vram_rev_b_txt_len    },
            { "193", reference_mappers_ntdec_tc_112_txt,          &reference_mappers_ntdec_tc_112_txt_len          },
            { "194", reference_mappers_tw_mmc3_vram_rev_c_txt,    &reference_mappers_tw_mmc3_vram_rev_c_txt_len    },
            { "195", reference_mappers_tw_mmc3_vram_rev_d_txt,    &reference_mappers_tw_mmc3_vram_rev_d_txt_len    },
            { "196", reference_mappers_unused_txt,                &reference_mappers_unused_txt_len                },
            { "197", reference_mappers_unused_txt,                &reference_mappers_unused_txt_len                },
            { "198", reference_mappers_tw_mmc3_vram_rev_e_txt,    &reference_mappers_tw_mmc3_vram_rev_e_txt_len    },
            { "199", reference_mappers_unused_txt,                &reference_mappers_unused_txt_len                },
            { "200", reference_mappers_unused_txt,                &reference_mappers_unused_txt_len                },
            { "201", reference_mappers_unused_txt,                &reference_mappers_unused_txt_len                },
            { "202", reference_mappers_unused_txt,                &reference_mappers_unused_txt_len                },
            { "203", reference_mappers_unused_txt,                &reference_mappers_unused_txt_len                },
            { "204", reference_mappers_unused_txt,                &reference_mappers_unused_txt_len                },
            { "205", reference_mappers_unused_txt,                &reference_mappers_unused_txt_len                },
            { "206", reference_mappers_namcot_108_rev_c_txt,      &reference_mappers_namcot_108_rev_c_txt_len      },
            { "207", reference_mappers_taito_x1_005_rev_b_txt,    &reference_mappers_taito_x1_005_rev_b_txt_len    },
            { "208", reference_mappers_unused_txt,                &reference_mappers_unused_txt_len                },
            { "209", reference_mappers_unused_txt,                &reference_mappers_unused_txt_len                },
            { "210", reference_mappers_unused_txt,                &reference_mappers_unused_txt_len                },
            { "211", reference_mappers_unused_txt,                &reference_mappers_unused_txt_len                },
            { "212", reference_mappers_unused_txt,                &reference_mappers_unused_txt_len                },
            { "213", reference_mappers_unused_txt,                &reference_mappers_unused_txt_len                },
            { "214", reference_mappers_unused_txt,                &reference_mappers_unused_txt_len                },
            { "215", reference_mappers_unused_txt,                &reference_mappers_unused_txt_len                },
            { "216", reference_mappers_unused_txt,                &reference_mappers_unused_txt_len                },
            { "217", reference_mappers_unused_txt,                &reference_mappers_unused_txt_len                },
            { "218", reference_mappers_unused_txt,                &reference_mappers_unused_txt_len                },
            { "219", reference_mappers_unla9746_txt,              &reference_mappers_unla9746_txt_len              },
            { "220", reference_mappers_debug_mapper_txt,          &reference_mappers_debug_mapper_txt_len          },
            { "221", reference_mappers_unln625092_txt,            &reference_mappers_unln625092_txt_len            },
            { "222", reference_mappers_unused_txt,                &reference_mappers_unused_txt_len                },
            { "223", reference_mappers_unused_txt,                &reference_mappers_unused_txt_len                },
            { "224", reference_mappers_unused_txt,                &reference_mappers_unused_txt_len                },
            { "225", reference_mappers_unused_txt,                &reference_mappers_unused_txt_len                },
            { "226", reference_mappers_bmc_22_20_in_1_txt,        &reference_mappers_bmc_22_20_in_1_txt_len        },
            { "227", reference_mappers_unused_txt,                &reference_mappers_unused_txt_len                },
            { "228", reference_mappers_unused_txt,                &reference_mappers_unused_txt_len                },
            { "229", reference_mappers_unused_txt,                &reference_mappers_unused_txt_len                },
            { "230", reference_mappers_bmc_contra_22_in_1_txt,    &reference_mappers_bmc_contra_22_in_1_txt_len    },
            { "231", reference_mappers_unused_txt,                &reference_mappers_unused_txt_len                },
            { "232", reference_mappers_bmc_quattro_txt,           &reference_mappers_bmc_quattro_txt_len           },
            { "233", reference_mappers_bmc_22_20_in_1_rst_txt,    &reference_mappers_bmc_22_20_in_1_rst_txt_len    },
            { "234", reference_mappers_bmc_maxi_txt,              &reference_mappers_bmc_maxi_txt_len              },
            { "235", reference_mappers_unused_txt,                &reference_mappers_unused_txt_len                },
            { "236", reference_mappers_unused_txt,                &reference_mappers_unused_txt_len                },
            { "237", reference_mappers_unused_txt,                &reference_mappers_unused_txt_len                },
            { "238", reference_mappers_unl6035052_txt,            &reference_mappers_unl6035052_txt_len            },
            { "239", reference_mappers_unused_txt,                &reference_mappers_unused_txt_len                },
            { "240", reference_mappers_unused_txt,                &reference_mappers_unused_txt_len                },
            { "241", reference_mappers_unused_txt,                &reference_mappers_unused_txt_len                },
            { "242", reference_mappers_unused_txt,                &reference_mappers_unused_txt_len                },
            { "243", reference_mappers_s74ls374na_txt,            &reference_mappers_s74ls374na_txt_len            },
            { "244", reference_mappers_decathlon_txt,             &reference_mappers_decathlon_txt_len             },
            { "245", reference_mappers_unused_txt,                &reference_mappers_unused_txt_len                },
            { "246", reference_mappers_fong_shen_bang_txt,        &reference_mappers_fong_shen_bang_txt_len        },
            { "247", reference_mappers_unused_txt,                &reference_mappers_unused_txt_len                },
            { "248", reference_mappers_unused_txt,                &reference_mappers_unused_txt_len                },
            { "249", reference_mappers_unused_txt,                &reference_mappers_unused_txt_len                },
            { "250", reference_mappers_unused_txt,                &reference_mappers_unused_txt_len                },
            { "251", reference_mappers_unused_txt,                &reference_mappers_unused_txt_len                },
            { "252", reference_mappers_san_guo_zhi_pirate_txt,    &reference_mappers_san_guo_zhi_pirate_txt_len    },
            { "253", reference_mappers_dragon_ball_pirate_txt,    &reference_mappers_dragon_ball_pirate_txt_len    },
            { "254", reference_mappers_unused_txt,                &reference_mappers_unused_txt_len                },
            { "255", reference_mappers_unused_txt,                &reference_mappers_unused_txt_len                }
        }
    }
};

#endif /* _REFERENCE_H */
