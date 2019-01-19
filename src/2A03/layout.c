// layout.c
// Author: hyan23
// Date: 2017.08.21

#include "header.h"
#include "../6502/memory.h"
#include "../mmc/mmc.h"
#include "../input/input.h"
#include "../2C02/2C02.h"
#include "../2C02/ppu.h"
#include "2A03.h"
#include "layout.h"
#include "pAPU.h"

byte cnes_layout_readbyte(cnes_memory_s* memory, word addr)
{
    cnes_layout_s* layout = (cnes_layout_s*) memory->layout;
    if (addr >= CNES_2A03_LOWER_BANK) {
        return layout->lower[CNES_LAYOUT_BANK_WRAP(addr)];
    } else if (addr >= CNES_2A03_SRAM) {
        return layout->SRAM[CNES_LAYOUT_SRAM_WRAP(addr)];
    } else if (addr >= CNES_2A03_EXPANSION) {
        return layout->expansion[CNES_LAYOUT_EXPANSION_WRAP(addr)];
    } else if (addr >= CNES_2A03_REGISTER2) {
        cnes_input_s* input = (cnes_input_s*) CNES_MEMORY_INPUT(*memory);
        cnes_pAPU_apu_s* apu = (cnes_pAPU_apu_s*) CNES_MEMORY_APU(*memory);
        byte* dest = &layout->REGISTER2[CNES_LAYOUT_REGISTER2_WRAP(addr)];
        switch (addr) {
            case CNES_2A03_PORT1:
                return *dest = cnes_input_read(input, PORT1);
            case CNES_2A03_PORT2:
                return *dest = cnes_input_read(input, PORT2);
            case CNES_2A03_APU_SIG:
                return *dest = cnes_pAPU_apu_read(apu, addr);
            default:
                return *dest;
        }
    } else if (addr >= CNES_2A03_REGISTER1) {
        cnes_ppu_s* ppu = (cnes_ppu_s*) CNES_MEMORY_PPU(*memory);
        word raddr = CNES_2A03_REGISTER1 + CNES_LAYOUT_REGISTER1_WRAP(addr);
        byte temp = layout->REGISTER1[CNES_LAYOUT_REGISTER1_WRAP(addr)] = 
            cnes_ppu_read(ppu, raddr);
        return temp;
    } else /* if (addr >= CNES_2A03_ZERO_PAGE) */ {
        return layout->internal[CNES_LAYOUT_INTERNAL_WRAP(addr)];
    }
}

word cnes_layout_readword(cnes_memory_s* memory, word addr)
{
    return cnes_layout_readbyte(memory, addr) | 
        cnes_layout_readbyte(memory, addr + 1) << 8;
}

static void dma_transfer(cnes_memory_s* memory, cnes_cpu_s* cpu, byte value)
{
    cpu->cycles -= 512;
    word addrh = value << 8;
    assert(addrh != CNES_2A03_REGISTER2);
    cnes_ppu_s* ppu = (cnes_ppu_s*) CNES_MEMORY_PPU(*memory);
    for (word i = 0; i < PPU_SPRITE_RAM; i ++) {
        cnes_ppu_write(ppu, CNES_2A03_SPRD, 
            cnes_layout_readbyte(memory, addrh + i));
    }
}

void cnes_layout_writebyte(cnes_memory_s* memory, word addr, byte value)
{
    cnes_layout_s* layout = (cnes_layout_s*) memory->layout;
    if (addr >= CNES_2A03_LOWER_BANK) {
        cnes_mmc_s* mmc = (cnes_mmc_s*) CNES_MEMORY_MMC(*memory);
        cnes_mmc_write(mmc, addr, value);
    } else if (addr >= CNES_2A03_SRAM) {
        layout->SRAM[CNES_LAYOUT_SRAM_WRAP(addr)] = value;
    } else if (addr >= CNES_2A03_EXPANSION) {
        layout->expansion[CNES_LAYOUT_EXPANSION_WRAP(addr)] = value;
    } else if (addr >= CNES_2A03_REGISTER2) {
        cnes_input_s* input = (cnes_input_s*) CNES_MEMORY_INPUT(*memory);
        cnes_pAPU_apu_s* apu = (cnes_pAPU_apu_s*) CNES_MEMORY_APU(*memory);
        switch (addr) {
            case CNES_2A03_SPR_DMA: {
                cnes_ppu_s* ppu = (cnes_ppu_s*) CNES_MEMORY_PPU(*memory);
                cnes_cpu_s* cpu = (cnes_cpu_s*) CNES_MEMORYP_CPU(*ppu->memory);
                dma_transfer(memory, cpu, value);
            }
            break;
            case CNES_2A03_PORT1:
                cnes_input_write(input, value);
                break;
            case CNES_2A03_PORT2:
            default:
                cnes_pAPU_apu_write(apu, addr, value);
                break;
        }
        layout->REGISTER2[CNES_LAYOUT_REGISTER2_WRAP(addr)] = value;
    } else if (addr >= CNES_2A03_REGISTER1) {
        cnes_ppu_s* ppu = (cnes_ppu_s*) CNES_MEMORY_PPU(*memory);
        word raddr = CNES_2A03_REGISTER1 + CNES_LAYOUT_REGISTER1_WRAP(addr);
        cnes_ppu_write(ppu, raddr, value);
        layout->REGISTER1[CNES_LAYOUT_REGISTER1_WRAP(addr)] = value;
    } else /* if (addr >= CNES_2A03_ZERO_PAGE) */ { 
        layout->internal[CNES_LAYOUT_INTERNAL_WRAP(addr)] = value;
    }
}

void cnes_layout_writeword(cnes_memory_s* memory, word addr, word value)
{
    cnes_layout_writebyte(memory, addr, value & 0xff);
    cnes_layout_writebyte(memory, addr + 1, value >> 8);
}