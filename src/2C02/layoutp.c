// layoutp.c
// Author: hyan23
// Date: 2017.08.24

#include "header.h"
#include "../6502/memory.h"
#include "../rom/rom.h"
#include "../mmc/mmc.h"
#include "2C02.h"
#include "ppu.h"
#include "layoutp.h"

#define PALETTE_ENTRY(entry)    ((entry) & 0x3f)

static word NAMETABLE_MIRROR(word addr, cnes_mirroring_t mirroring) 
{
    switch (mirroring) {
        case M_HORIZONAL:
            return (addr & 0x3ff) | ((addr >> 1) & 0x400);
            // return addr < 0x800 ? 
            //     addr & 0x3ff : 
            //     0x400 + (addr & 0x3ff);
        case M_VERTICAL:
            return addr & 0x7ff;
        case M_SINGLEL:
            return addr & 0x3ff;
        case M_SINGLEU:
            return 0x400 + (addr & 0x3ff);
        case M_FOURSCREEN:
            return addr;
        default:
            assert(FALSE);
    }
}

byte cnes_layoutp_readbyte(cnes_memory_s* memory, word addr)
{
    cnes_layoutp_s* layoutp = (cnes_layoutp_s*) memory->layout;
    cnes_ppu_s* ppu = (cnes_ppu_s*) CNES_MEMORYP_PPU(*memory);
    word paddr = CNES_LAYOUTP_ADDR_WRAP(addr);
    if (paddr >= CNES_2C02_PALETTE0) {
        return PALETTE_ENTRY(layoutp->palette0[
            CNES_LAYOUTP_PALETTE_MIRROR(
                CNES_LAYOUTP_PALETTE_WRAP(paddr))]);
    } else if (paddr >= CNES_2C02_NAMETABLE0) {
        return layoutp->nametable0[
            NAMETABLE_MIRROR(CNES_LAYOUTP_NAMETABLE_WRAP(paddr), 
                ppu->mirroring)];
    } else /* if (paddr >= CNES_2C02_PALETTE0) */ {
        byte value = layoutp->pattern0[paddr];
        switch (ppu->mapper) {
            case 9:
                cnes_mmc2_update(
                    (cnes_mmc_s*) CNES_MEMORYP_MMC(*memory), addr);
                break;
            case 10:
                cnes_mmc4_update(
                    (cnes_mmc_s*) CNES_MEMORYP_MMC(*memory), addr);
                break;
            default:
                break;
        }
        return value;
    }
}

word cnes_layoutp_readword(cnes_memory_s* memory, word addr)
{
    return cnes_layoutp_readbyte(memory, addr) | 
        cnes_layoutp_readbyte(memory, addr + 1) << 8;
}

void cnes_layoutp_writebyte(cnes_memory_s* memory, word addr, byte value)
{
    cnes_layoutp_s* layoutp = (cnes_layoutp_s*) memory->layout;
    cnes_ppu_s* ppu = (cnes_ppu_s*) CNES_MEMORYP_PPU(*memory);
    word paddr = CNES_LAYOUTP_ADDR_WRAP(addr);
    if (paddr >= CNES_2C02_PALETTE0) {
        layoutp->palette0[
            CNES_LAYOUTP_PALETTE_MIRROR(
                CNES_LAYOUTP_PALETTE_WRAP(paddr))] = 
                PALETTE_ENTRY(value);
    } else if (paddr >= CNES_2C02_NAMETABLE0) {
        layoutp->nametable0[
            NAMETABLE_MIRROR(CNES_LAYOUTP_NAMETABLE_WRAP(paddr), 
                ppu->mirroring)] = value;
    } else /* if (paddr >= CNES_2C02_PALETTE0) */ {
        layoutp->pattern0[paddr] = value;
    }
}

void cnes_layoutp_writeword(cnes_memory_s* memory, word addr, word value)
{
    cnes_layoutp_writebyte(memory, addr, value & 0xff);
    cnes_layoutp_writebyte(memory, addr + 1, value >> 8);
}