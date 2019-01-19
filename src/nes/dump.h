// dump.h
// Author: hyan23
// Date: 2018.05.28

#ifndef __DUMP_H__
#define __DUMP_H__

#include "nes.h"

typedef enum {
    D_TEXT, D_BINARY
}
cnes_dump_t;

#define CNES_DUMP_ITEMS     uint

#define DUMP_INPUT          1
#define DUMP_LAYOUT         2
#define DUMP_CPU            3
#define DUMP_LAYOUTP        4
#define DUMP_PPU            5
#define DUMP_MMC            6
#define DUMP_APU            7

#define DUMP_INPUT_MASK     (0x1 << DUMP_INPUT)
#define DUMP_LAYOUT_MASK    (0x1 << DUMP_LAYOUT)
#define DUMP_CPU_MASK       (0x1 << DUMP_CPU)
#define DUMP_LAYOUTP_MASK   (0x1 << DUMP_LAYOUTP)
#define DUMP_PPU_MASK       (0x1 << DUMP_PPU)
#define DUMP_MMC_MASK       (0x1 << DUMP_MMC)
#define DUMP_APU_MASK       (0x1 << DUMP_APU)
#define DUMP_ALL            (~0)

#define CNES_DUMP_MAGIC     "cnes"

#pragma pack(8)
typedef struct CNES_DUMP_HEADER
{
    char magic[5];
    CNES_DUMP_ITEMS items;;
    uint input_offset;
    uint layout_offset;
    uint cpu_offset;
    uint layoutp_offset;
    uint ppu_offset;
    uint mmc_offset;
    uint apu_offset;
    uint reserved0;
    uint reserved1;
    uint reserved2;
    uint reserved3;
    uint reserved4;
    uint reserved5;
    uint reserved6;
    uint reserved7;
}
cnes_dump_header_s;
#pragma pack()

extern error_t cnes_dump(const cnes_nes_s* nes, 
    cnes_dump_t type, CNES_DUMP_ITEMS items);
extern error_t cnes_load(cnes_nes_s* nes, cnes_dump_t type);

#endif /*__DUMP_H__ */