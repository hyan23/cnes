// ppu.c
// Author: hyan23
// Date: 2018.04.23

#include "header.h"
#include "../6502/cpu.h"
#include "../2A03/2A03.h"
#include "../mmc/mmc.h"
#include "layoutp.h"
#include "2C02.h"
#include "ppu.h"

static void sprite_evaluate(cnes_ppu_s* ppu, byte sprite_index)
{
    assert(sprite_index < PPU_SPRITE_ROOMS);
    if (ppu->stm.in_range >= PPU_SPRITES_SCANLINE) {
        PPU_OVERFLOW_SET(ppu);
        return;
    }
    sprite_t sprite = PPU_GET_SPRITE(*ppu, sprite_index);
    byte attr = PPU_SPRITE_ATTR(*sprite);
    sint cmp_result = ppu->scan_line - PPU_SPRITE_YC(*sprite);
    byte range = PPU_SPR_SIZE(*ppu) ? 15 : 7;
    if (BETWEEN(cmp_result, 0, range) && 
        PPU_SPRITE_YC(*sprite) < 240) {
        if (sprite_index == 0) {
            ppu->stm.found = TRUE;
        }
        struct STM_SPRITE* dest = &ppu->stm.sprite[ppu->stm.in_range];
        dest->attr = PPU_SPRITE_ATTR_PS(attr);
        dest->cmp_result = PPU_SPRITE_ATTR_VF(attr) ? 
            range - cmp_result : 
            cmp_result;
        dest->hf = PPU_SPRITE_ATTR_HF(attr);
        dest->priority = PPU_SPRITE_ATTR_PRIORITY(attr);
        dest->xc = PPU_SPRITE_XC(*sprite);
        dest->tile_index = PPU_SPRITE_TILE_INDEX(*sprite);
        ppu->stm.in_range ++;
    }
}

static void fetch_bg_tile(cnes_ppu_s* ppu)
{
    word pattern_tbl = PPU_BG_TILE_TBL(*ppu);
    byte tile_index = cnes_layoutp_readbyte(ppu->memory, 
        CNES_2C02_NAMETABLE0 + (ppu->address & 0xfff));
    byte fineY = PPU_TEMP_GET_FINEY(ppu->address);
    ppu->b1 = cnes_layoutp_readbyte(ppu->memory, 
        pattern_tbl + tile_index * 16 + fineY);
    ppu->b2 = cnes_layoutp_readbyte(ppu->memory, 
        pattern_tbl + tile_index * 16 + 8 + fineY);
    byte tileX = PPU_TEMP_GET_TILEX(ppu->address);
    byte tileY = PPU_TEMP_GET_TILEY(ppu->address);
    ppu->attr = cnes_layoutp_readbyte(ppu->memory, 
        (CNES_2C02_NAMETABLE0 | (PPU_TEMP_GET_NN(ppu->address) << 10)) + 
        CNES_LAYOUTP_NAME_TABLE0 + tileY / 4 * 8 + tileX / 4);
    BOOL top = tileY % 4 < 2;
    BOOL left = tileX % 4 < 2;
    if (!top)   ppu->attr >>= 4;
    if (!left)  ppu->attr >>= 2;
    ppu->attr &= 0x3;
}

static void load_shift_registers(cnes_ppu_s* ppu)
{
    // ppu->shift1 &= 0xff00;
    // ppu->shift2 &= 0xff00;
    // ppu->shift3 &= 0xff00;
    // ppu->shift4 &= 0xff00;
    ppu->shift1 |= ppu->b1;
    ppu->shift2 |= ppu->b2;
    ppu->latch = ppu->attr;
}

static void shift_shift_registers(cnes_ppu_s* ppu)
{
    ppu->shift1 <<= 1;
    ppu->shift2 <<= 1;
    ppu->shift3 <<= 1;
    ppu->shift4 <<= 1;
    ppu->shift3 |= test_bit(ppu->latch, 0) ? 1 : 0;
    ppu->shift4 |= test_bit(ppu->latch, 1) ? 1 : 0;
}

static void inc_tileX(cnes_ppu_s* ppu)
{
    byte tileX = PPU_TEMP_GET_TILEX(ppu->address);
    if (tileX < 31) {
        PPU_TEMP_SET_TILEX(&ppu->address, tileX + 1);
    } else {
        PPU_TEMP_SET_TILEX(&ppu->address, 0);
        PPU_TEMP_FLIP_NNL(&ppu->address);
    }
}

// fineY is increased at the end of a scanline
static void inc_fineY(cnes_ppu_s* ppu)
{
    byte fineY = PPU_TEMP_GET_FINEY(ppu->address);
    if (fineY < 7) {
        PPU_TEMP_SET_FINEY(&ppu->address, fineY + 1);
    } else {
        PPU_TEMP_SET_FINEY(&ppu->address, 0);
        byte tileY = PPU_TEMP_GET_TILEY(ppu->address);
        // Normally, tileY wraps from 29 to 0, and the high bit of NN 
        // will be flipped, but if tileY is set to 30 or 31 via 
        // VRAM address port by hand, it will wrap from 31 to 0, 
        // in this case, NN will not be flipped
        if (tileY == 29) {
            PPU_TEMP_SET_TILEY(&ppu->address, 0);
            PPU_TEMP_FLIP_NNH(&ppu->address);
        } else if (tileY == 31) {
            PPU_TEMP_SET_TILEY(&ppu->address, 0);
        } else {
            PPU_TEMP_SET_TILEY(&ppu->address, tileY + 1);
        }
    }
}

static byte flip_byte(byte value)
{
    static byte flip_half[16] = {
        0x00, 0x08, 0x04, 0x0c, 0x02, 0x0a, 0x06, 0x0e, 
        0x01, 0x09, 0x05, 0x0d, 0x03, 0x0b, 0x07, 0x0f
    };
    byte result = 0;
    result |= flip_half[value & 0x0f];
    result <<= 4;
    value >>= 4;
    return result | flip_half[value & 0x0f];
}

static void fetch_sprite_tile(cnes_ppu_s* ppu, byte sprite_index /* 0 - 7 */)
{
    if (sprite_index < ppu->stm.in_range) {
        struct STM_SPRITE* pt = &ppu->stm.sprite[sprite_index];
        struct SBM_BUFFER* pb = &ppu->sbm.buffer[sprite_index];
        pb->attr = pt->attr;
        pb->priority = pt->priority;
        pb->dc = pt->xc;
        word pattern_tbl = PPU_SPR_SIZE(*ppu) ? 
            (pt->tile_index & 0x1 ? 0x1000 : 0) : 
            PPU_SPR_TILE_TBL(*ppu);
        byte tile_index = PPU_SPR_SIZE(*ppu) ? 
            (pt->tile_index & 0xfe) | (pt->cmp_result >> 3) : 
            pt->tile_index;
        byte b1 = cnes_layoutp_readbyte(ppu->memory, pattern_tbl + 
            tile_index * 16 + (pt->cmp_result & 0x7));
        byte b2 = cnes_layoutp_readbyte(ppu->memory, pattern_tbl + 
            tile_index * 16 + 8 + (pt->cmp_result & 0x7));
        pb->shift1 = pt->hf ? flip_byte(b1) : b1;
        pb->shift2 = pt->hf ? flip_byte(b2) : b2;
    }
}

static void render_pixel(cnes_ppu_s* ppu)
{
    byte bg = 0;
    if (PPU_BG_VISIBLE(*ppu) && (!PPU_BG_CLIP(*ppu) || ppu->pixel >= 8)) {
        if (test_bit(ppu->shift1, 15 - ppu->fineX)) set_bit(&bg, 0);
        if (test_bit(ppu->shift2, 15 - ppu->fineX)) set_bit(&bg, 1);
        if (test_bit(ppu->shift3, 7 - ppu->fineX)) set_bit(&bg, 2);
        if (test_bit(ppu->shift4, 7 - ppu->fineX)) set_bit(&bg, 3);
        if (PPU_TRANSPARENT(bg)) {
            bg = 0;
        }
    }
    byte spr = 0;
    byte sprite_index = PPU_SPRITE_ROOMS;
    for (int i = ppu->num - 1; i >= 0; i --) {
        if (ppu->sbm.buffer[i].dc > 0) {
            ppu->sbm.buffer[i].dc --;
        } else {
            if (ppu->sbm.buffer[i].shift < 8) {
                if (PPU_SPR_VISIBLE(*ppu) && 
                    (!PPU_SPR_CLIP(*ppu) || ppu->pixel >= 8)) {
                    byte temp = 0;
                    if (test_bit(ppu->sbm.buffer[i].attr, 1))   set_bit(&temp, 3);
                    if (test_bit(ppu->sbm.buffer[i].attr, 0))   set_bit(&temp, 2);
                    if (test_bit(ppu->sbm.buffer[i].shift2, 7)) set_bit(&temp, 1);
                    if (test_bit(ppu->sbm.buffer[i].shift1, 7)) set_bit(&temp, 0);
                    if (!PPU_TRANSPARENT(temp)) {
                        spr = temp;
                        sprite_index = i;
                    }
                }
                ppu->sbm.buffer[i].shift2 <<= 1;
                ppu->sbm.buffer[i].shift1 <<= 1;
                ppu->sbm.buffer[i].shift ++;
            }
        }
    }
    if (!PPU_TRANSPARENT(spr)) {
        if (!PPU_TRANSPARENT(bg) && 
            ppu->sbm.primary_present && sprite_index == 0 && 
            ppu->pixel != 255) {
            PPU_HIT_SET(ppu);
        }
        if (!ppu->sbm.buffer[sprite_index].priority || PPU_TRANSPARENT(bg)) {
            bg = CNES_LAYOUTP_PALETTE0 + spr;
        }
    }
    ppu->image[ppu->scan_line][ppu->pixel] = 
        cnes_layoutp_readbyte(ppu->memory, CNES_2C02_PALETTE0 + bg);
    ppu->pixel ++;
}

static void ppu_scanline(cnes_ppu_s* ppu, BOOL dummy)
{
    ppu->pixel = 0;
    // ppu_cycle 0 is idle
    for (uint ppu_cycle = 1; ppu_cycle < PPU_SCANLINE_CYCLES - 1; ppu_cycle ++) {
        if (BETWEEN(ppu_cycle, 1, 256)) {
            /* Sprite evaluation */
            if (ppu_cycle == 1) {
                ppu->num = ppu->stm.in_range;
                ppu->stm.in_range = 0;
                ppu->stm.found = 0;
            }
            if (ppu_cycle % 4 == 0) {
                sprite_evaluate(ppu, ppu_cycle / 4 - 1);
            }
            /* Reload shift registers */
            if (ppu->reload) {
                load_shift_registers(ppu);
                ppu->reload = FALSE;
            }
            /* Fetch data */
            if (ppu_cycle % 8 == 0) {
                fetch_bg_tile(ppu);
                ppu->reload = TRUE;
                if (ppu_cycle < 256) {
                    inc_tileX(ppu);
                } else {
                    inc_fineY(ppu);
                    ppu->address = (ppu->address & 0xfbff) | (ppu->temp & 0x400);
                    PPU_TEMP_SET_TILEX(&ppu->address, 
                        PPU_TEMP_GET_TILEX(ppu->temp));
                }
            }
            /* Render pixel */
            if (!dummy) {
                render_pixel(ppu);
            }
            shift_shift_registers(ppu);
        } else if (BETWEEN(ppu_cycle, 257, 320)) {
            if (ppu_cycle == 257) {
                cnes_mmc3_update(
                    (cnes_mmc_s*) CNES_MEMORYP_MMC(*ppu->memory));
                if (!dummy) {
                    memset(ppu->sbm.buffer, 0, sizeof (ppu->sbm.buffer));
                    ppu->sbm.primary_present = ppu->stm.found;
                }
            }
            if (dummy && BETWEEN(ppu_cycle, 280, 304)) {
                ppu->address = (ppu->address & 0xf7ff) | (ppu->temp & 0x800);
                PPU_TEMP_SET_TILEY(&ppu->address, PPU_TEMP_GET_TILEY(ppu->temp));
                PPU_TEMP_SET_FINEY(&ppu->address, PPU_TEMP_GET_FINEY(ppu->temp));
            }
            if (ppu_cycle % 8 == 0) {
                fetch_sprite_tile(ppu, ppu_cycle / 8 - 33);
            }
        } else if (BETWEEN(ppu_cycle, 321, 336)) {
            if (ppu->reload) {
                load_shift_registers(ppu);
                ppu->reload = FALSE;
            }
            if (ppu_cycle % 8 == 0) {
                fetch_bg_tile(ppu);
                ppu->reload = TRUE;
                inc_tileX(ppu);
            }
            shift_shift_registers(ppu);
        } else if (BETWEEN(ppu_cycle, 337, 340)) {
            if (ppu_cycle == 337) {
                
            }
        }
    }
}

BOOL cnes_ppu_line(cnes_ppu_s* ppu)
{
    if (BETWEEN(ppu->line, 0, 239)) {
        ppu->scan_line = ppu->line;
        if (PPU_ONE_VISIBLE(*ppu)) {
            ppu_scanline(ppu, FALSE);
        }
    } else if (BETWEEN(ppu->line, 240, 260)) {
        if (ppu->line == 241) {
            PPU_VBLANK_SET(ppu);
            if (PPU_NMI_ENABLED(*ppu)) {
                cnes_cpu_signal(
                    (cnes_cpu_s*) CNES_MEMORYP_CPU(*ppu->memory), INT_NMI);
            }
        }
    } else {    /* 261 */
        PPU_VBLANK_CLEAR(ppu);
        PPU_HIT_CLEAR(ppu);
        PPU_OVERFLOW_CLEAR(ppu);
        if (PPU_ONE_VISIBLE(*ppu)) {
            ppu->scan_line = ppu->line;
            ppu_scanline(ppu, TRUE);
        }
    }
    if (ppu->line < 261) {
        ppu->line ++;
    } else {
        ppu->line = 0;
    }
    return ppu->line == 241;
}

byte cnes_ppu_read(cnes_ppu_s* ppu, word addr)
{
    assert(addr <= 0x3fff);
    switch (addr) {
        case CNES_2A03_PPUW1:
            return ppu->control_register1;
        case CNES_2A03_PPUW2:
            return ppu->control_register2;
        case CNES_2A03_PPUR1: {
            byte value = ppu->status_register;
            PPU_VBLANK_CLEAR(ppu);
            PPU_TOGGLE_CLEAR(&ppu->toggle);
            return value;
        }
        case CNES_2A03_SPRA:    /* write only!! */
            return ppu->sprite_index;
        case CNES_2A03_SPRD:
            return ppu->sprite_ram[ppu->sprite_index];
        case CNES_2A03_VRAMAL:  /* write only!! */
            return ppu->vramal;
        case CNES_2A03_VRAMAH:  /* write only!! */
            return ppu->vramah;
        case CNES_2A03_VRAMD: {
            ppu->value = ppu->read_cache;
            if (CNES_LAYOUTP_ADDR_WRAP(ppu->address) >= CNES_2C02_PALETTE0) {
                ppu->value = ppu->read_cache;
            } else {
                ppu->read_cache = cnes_layoutp_readbyte(ppu->memory, ppu->address);
            }
            ppu->address += PPU_ADDR_INC(*ppu);
            return ppu->value;
        }
        default:
            assert(FALSE);
            break;
    }
}

void cnes_ppu_write(cnes_ppu_s* ppu, word addr, byte value)
{
    assert(addr <= 0x3fff);
    switch (addr) {
        case CNES_2A03_PPUW1:
            PPU_TEMP_SET_NN(&ppu->temp, value & 0x3);
            ppu->control_register1 = value;
            if (PPU_NMI_ENABLED(*ppu) && PPU_VBLANK(*ppu)) {
                // cnes_cpu_signal((cnes_cpu_s*) CNES_MEMORYP_CPU(*ppu->memory), 
                //     INT_NMI);
            }
            break;
        case CNES_2A03_PPUW2:
            ppu->control_register2 = value;
            break;
        case CNES_2A03_PPUR1:       /* read only!! */
            ppu->status_register = value;
            break;
        case CNES_2A03_SPRA:
            ppu->sprite_index = value;
            break;
        case CNES_2A03_SPRD:
            ppu->sprite_ram[ppu->sprite_index] = value;
            ppu->sprite_index += 1;
            break;
        case CNES_2A03_VRAMAL:
            ppu->vramal = value;
            if (!ppu->toggle) {
                PPU_TEMP_SET_TILEX(&ppu->temp, (value >> 3) /*& 0x1f */);
                PPU_SET_FINEX(ppu, value & 0x7);
                PPU_TOGGLE_SET(&ppu->toggle);
            } else {
                PPU_TEMP_SET_TILEY(&ppu->temp, value >> 3);
                PPU_TEMP_SET_FINEY(&ppu->temp, value & 0x7);
                PPU_TOGGLE_CLEAR(&ppu->toggle);
            }
            break;
        case CNES_2A03_VRAMAH:
            ppu->vramah = value;
            if (!ppu->toggle) {
                PPU_TEMP_SET_TILEYH(&ppu->temp, value & 0x3);
                PPU_TEMP_SET_NN(&ppu->temp, (value >> 2) & 0x3);
                PPU_TEMP_SET_FINEY(&ppu->temp, (value >> 4) & 0x3);
                PPU_TOGGLE_SET(&ppu->toggle);
            } else {
                PPU_TEMP_SET_TILEX(&ppu->temp, value & 0x1f);
                PPU_TEMP_SET_TILEYL(&ppu->temp, (value >> 5) & 0x7);
                ppu->address = ppu->temp;
                PPU_TOGGLE_CLEAR(&ppu->toggle);
            }
            break;
        case CNES_2A03_VRAMD:
            if (CNES_LAYOUTP_ADDR_WRAP(ppu->address) >= CNES_2C02_PALETTE0) {
                cnes_layoutp_writebyte(ppu->memory, ppu->address, value);
            } else {
                cnes_layoutp_writebyte(ppu->memory, ppu->address, 
                    /* ppu->write_cache */ value);
            }
            ppu->write_cache = value;
            ppu->address += PPU_ADDR_INC(*ppu);
            break;
        default:
            assert(FALSE);
            break;
    }
}

void cnes_ppu_dumpb(FILE* fout, const cnes_ppu_s* ppu)
{
    fwrite(ppu, sizeof (cnes_ppu_s), 1, fout);
}

void cnes_ppu_loadb(cnes_ppu_s* ppu, FILE* fin)
{
    cnes_memory_s* memory = ppu->memory;
    fread(ppu, sizeof (cnes_ppu_s), 1, fin);
    ppu->memory = memory;
}