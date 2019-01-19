// rom.h
// Author: hyan23
// Date: 2017.08.01

#ifndef __ROM_H__
#define __ROM_H__

#include "header.h"
#include "common.h"
#include "cartridge.h"

#pragma pack(1)
typedef struct CNES_HEADER
{
    byte    magic[4];       /* 'N', 'E', 'S', 0x1a */
    byte    prg_pgs;        /* 16k PRG-ROM page count */
    byte    chr_pgs;        /* 8k CHR-ROM page count */
    byte    control1;       /* ROM Control Byte #1 */
    byte    control2;       /* ROM Control Byte #2 */
    byte    reserved[8];    /* 0x0 */
} 
cnes_header_s;
#pragma pack()

#define PRG_PAGE_SIZE   0x4000
#define CHR_PAGE_SIZE   0x2000

typedef struct CNES_PRG
{
    byte*   rom;
    uint    offset;
    uint    size;
}
cnes_prg_s;

typedef cnes_prg_s cnes_chr_s;

// M_SINGLE, Lower Bank
// M_SINGLE, Upper Bank
typedef enum CNES_MIRRORING {
    M_HORIZONAL,    M_VERTICAL, M_SINGLEL,  M_SINGLEU,  M_FOURSCREEN
}
cnes_mirroring_t;

typedef struct CNES_ROM
{
    cnes_header_s       header;
    cnes_mirroring_t    mirroring;
    byte                mapper;
    BOOL                sram_present;
    cnes_prg_s          prg;
    cnes_chr_s          chr;
}
cnes_rom_s;

extern error_t cnes_rom_open(cnes_rom_s* rom, cnes_cartridge_s* cartridge);
extern void cnes_rom_close(cnes_rom_s* rom);
extern void cnes_rom_dump(const cnes_rom_s* rom);

#endif /* __ROM_H__ */