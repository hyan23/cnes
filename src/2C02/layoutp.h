// layoutp.h
// Author: hyan23
// Date: 2017.08.24

#ifndef __LAYOUTP_H__
#define __LAYOUTP_H__

#include "header.h"
#include "../6502/memory.h"

#define CNES_LAYOUTP_PATTERN0       0x1000
#define CNES_LAYOUTP_PATTERN1       CNES_LAYOUTP_PATTERN0
#define CNES_LAYOUTP_NAME_TABLE0    0x3c0
#define CNES_LAYOUTP_ATTRIBUTE0     0x40
#define CNES_LAYOUTP_NAME_TABLE1    CNES_LAYOUTP_NAME_TABLE0
#define CNES_LAYOUTP_ATTRIBUTE1     CNES_LAYOUTP_ATTRIBUTE0
#define CNES_LAYOUTP_NAME_TABLE2    CNES_LAYOUTP_NAME_TABLE0
#define CNES_LAYOUTP_ATTRIBUTE2     CNES_LAYOUTP_ATTRIBUTE0
#define CNES_LAYOUTP_NAME_TABLE3    CNES_LAYOUTP_NAME_TABLE0
#define CNES_LAYOUTP_ATTRIBUTE3     CNES_LAYOUTP_ATTRIBUTE0
#define CNES_LAYOUTP_PALETTE0       0x10
#define CNES_LAYOUTP_PALETTE1       CNES_LAYOUTP_PALETTE0

#define CNES_LAYOUTP_ADDR_WRAP(addr)        ((addr) & 0x3fff)
#define CNES_LAYOUTP_NAMETABLE_WRAP(addr)   ((addr) & (4 * \
                (CNES_LAYOUTP_NAME_TABLE0 + CNES_LAYOUTP_ATTRIBUTE0) - 1))
#define CNES_LAYOUTP_PALETTE_WRAP(addr)     ((addr) & ((2 * \
                CNES_LAYOUTP_PALETTE0) - 1))
#define CNES_LAYOUTP_PALETTE_MIRROR(addr)   (((addr) & 0x3) == 0 ? \
    ((addr) & 0xf) : (addr))

#pragma pack(1)
typedef struct CNES_LAYOUTP
{
    byte    pattern0[CNES_LAYOUTP_PATTERN0];
    byte    pattern1[CNES_LAYOUTP_PATTERN1];
    byte    nametable0[CNES_LAYOUTP_NAME_TABLE0];
    byte    attribute0[CNES_LAYOUTP_ATTRIBUTE0];
    byte    nametable1[CNES_LAYOUTP_NAME_TABLE1];
    byte    attribute1[CNES_LAYOUTP_ATTRIBUTE1];
    byte    nametable2[CNES_LAYOUTP_NAME_TABLE2];   /* unavailable in real nes */
    byte    attribute2[CNES_LAYOUTP_ATTRIBUTE2];
    byte    nametable3[CNES_LAYOUTP_NAME_TABLE3];   /* unavailable in real nes */
    byte    attribute3[CNES_LAYOUTP_ATTRIBUTE3];
    /* Name table mirror * 1 */
    byte    palette0[CNES_LAYOUTP_PALETTE0];    /* image palette */
    byte    palette1[CNES_LAYOUTP_PALETTE1];    /* sprite palette */
    /* Mirrors of above * 3 */
}
cnes_layoutp_s;
#pragma pack()

#define CNES_MEMORYP_CPU(memory)    ((memory).ref1)
#define CNES_MEMORYP_PPU(memory)    ((memory).ref2)
#define CNES_MEMORYP_MMC(memory)    ((memory).ref3)

extern byte cnes_layoutp_readbyte(cnes_memory_s* memory, word addr);
extern word cnes_layoutp_readword(cnes_memory_s* memory, word addr);
extern void cnes_layoutp_writebyte(cnes_memory_s* memory, 
    word addr, byte value);
extern void cnes_layoutp_writeword(cnes_memory_s* memory, 
    word addr, word value);

#endif /* __LAYOUTP_H__ */