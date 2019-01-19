// layout.h
// Author: hyan23
// Date: 2017.08.21

#ifndef __LAYOUT_H__
#define __LAYOUT_H__

#include "header.h"
#include "../6502/memory.h"

#define CNES_LAYOUT_INTERNAL        0x0800
#define CNES_LAYOUT_REGISTER1       0x0008
#define CNES_LAYOUT_REGISTER2       0x0020
#define CNES_LAYOUT_EXPANSION       0x1fe0
#define CNES_LAYOUT_SRAM            0x2000
#define CNES_LAYOUT_LOWER_BANK      0x4000
#define CNES_LAYOUT_UPPER_BANK      CNES_LAYOUT_LOWER_BANK

#define CNES_LAYOUT_BANK_WRAP(addr)         ((addr) & ((2 *     \
                                CNES_LAYOUT_LOWER_BANK) - 1))
#define CNES_LAYOUT_EXPANSION_WRAP(addr)    (((addr) - CNES_2A03_EXPANSION) &   \
                                (CNES_LAYOUT_EXPANSION - 1))
#define CNES_LAYOUT_SRAM_WRAP(addr)         ((addr) & (CNES_LAYOUT_SRAM - 1))
#define CNES_LAYOUT_REGISTER2_WRAP(addr)    ((addr) & (CNES_LAYOUT_REGISTER2 - 1))
#define CNES_LAYOUT_REGISTER1_WRAP(addr)    ((addr) & (CNES_LAYOUT_REGISTER1 - 1))
#define CNES_LAYOUT_INTERNAL_WRAP(addr)     ((addr) & (CNES_LAYOUT_INTERNAL - 1))

#pragma pack(1)
typedef struct CNES_LAYOUT
{
    byte internal[CNES_LAYOUT_INTERNAL];
    byte REGISTER1[CNES_LAYOUT_REGISTER1];
    byte REGISTER2[CNES_LAYOUT_REGISTER2];
    byte expansion[CNES_LAYOUT_EXPANSION];
    byte SRAM[CNES_LAYOUT_SRAM];
    byte lower[CNES_LAYOUT_LOWER_BANK];
    byte upper[CNES_LAYOUT_UPPER_BANK];
}
cnes_layout_s;
#pragma pack()

#define CNES_MEMORY_MMC(memory)     ((memory).ref1)
#define CNES_MEMORY_INPUT(memory)   ((memory).ref2)
#define CNES_MEMORY_PPU(memory)     ((memory).ref3)
#define CNES_MEMORY_APU(memory)     ((memory).ref4)

extern byte cnes_layout_readbyte(cnes_memory_s* memory, word addr);
extern word cnes_layout_readword(cnes_memory_s* memory, word addr);
extern void cnes_layout_writebyte(cnes_memory_s* memory, 
    word addr, byte value);
extern void cnes_layout_writeword(cnes_memory_s* memory, 
    word addr, word value);

#endif /* __LAYOUT_H__ */