// mapper4.c
// Author: hyan23
// Date: 2018.05.03

#include "mmc.h"

// MMC3 internal registers
#define reload_value        r0  /* IRQ counter reload value */
#define counter             r1
#define reload              r2  /* reload indicator, reload when =1 */
#define irq_enabled         r3

#define control         r4

#define CONTROL_BANK_REGISTER(control)  ((control) & 0x7)
#define CONTROL_PRG_MODE(control)       (((control) >> 6) & 0x1)
#define CONTROL_CHR_MODE(control)       (((control) >> 7) & 0x1)

static void 
swap_chr_bank(cnes_mmc_s* mmc, uint reg, uint mode, byte value)
{
    assert(mode == 0 || mode == 1);
    if (mmc->rom->header.chr_pgs == 0) {
        return;
    }
    switch (reg) {
        case 0: {
            mmc_oneKbank_t src = MMC_GET_1KCHR_BANK(mmc, value & 0xfe);
            MMC_SWITCH_1K_BANK(RAW_PTR(mmc->layoutp->pattern0) + 
                mode * 0x1000, src);
            src = MMC_GET_1KCHR_BANK(mmc, value | 0x1);
            MMC_SWITCH_1K_BANK(RAW_PTR(mmc->layoutp->pattern0) + 
                0x400 + mode * 0x1000, src);
        }
        break;
        case 1: {
            mmc_oneKbank_t src = MMC_GET_1KCHR_BANK(mmc, value & 0xfe);
            MMC_SWITCH_1K_BANK(RAW_PTR(mmc->layoutp->pattern0) + 
                0x800 + mode * 0x1000, src);
            src = MMC_GET_1KCHR_BANK(mmc, value | 0x1);
            MMC_SWITCH_1K_BANK(RAW_PTR(mmc->layoutp->pattern0) + 
                0xc00 + mode * 0x1000, src);
        }
        break;
        case 2: {
            mmc_oneKbank_t src = MMC_GET_1KCHR_BANK(mmc, value);
            MMC_SWITCH_1K_BANK(RAW_PTR(mmc->layoutp->pattern0) + 
                0x1000 - mode * 0x1000, src);
        }
        break;
        case 3: {
            mmc_oneKbank_t src = MMC_GET_1KCHR_BANK(mmc, value);
            MMC_SWITCH_1K_BANK(RAW_PTR(mmc->layoutp->pattern0) + 
                0x1400 - mode * 0x1000, src);
        }
        break;
        case 4: {
            mmc_oneKbank_t src = MMC_GET_1KCHR_BANK(mmc, value);
            MMC_SWITCH_1K_BANK(RAW_PTR(mmc->layoutp->pattern0) + 
                0x1800 - mode * 0x1000, src);
        }
        break;
        case 5: {
            mmc_oneKbank_t src = MMC_GET_1KCHR_BANK(mmc, value);
            MMC_SWITCH_1K_BANK(RAW_PTR(mmc->layoutp->pattern0) + 
                0x1c00 - mode * 0x1000, src);
        }
        break;
        default:
            assert(FALSE);
            break;
    }
}

static void 
swap_prg_bank(cnes_mmc_s* mmc, uint reg, uint mode, uint value)
{
    assert(mode == 0 || mode == 1);
    switch (reg) {
        case 6: {
            mmc_eightKbank_t src = MMC_GET_8KPRG_BANK(mmc, value);
            MMC_SWITCH_8K_BANK(RAW_PTR(mmc->layout->lower) + mode * 0x4000, src);
        }
        break;
        case 7: {
            mmc_eightKbank_t src = MMC_GET_8KPRG_BANK(mmc, value);
            MMC_SWITCH_8K_BANK(RAW_PTR(mmc->layout->lower) + 0x2000, src);
        }
        break;
        default:
            assert(FALSE);
            break;
    }
}

void CNES_MAPPER_INIT_FUNC_NAME(4)(cnes_mmc_s* mmc)
{
    mmc->name = "MMC3";
    // Not sure
    mmc_sixteenKbank_t src = MMC_GET_16KPRG_BANK(mmc, 
        mmc->rom->header.prg_pgs - 1);
    MMC_SWITCH_16K_BANK(mmc->layout->lower, src);
    MMC_SWITCH_16K_BANK(mmc->layout->upper, src);
}

byte CNES_MAPPER_READ_FUNC_NAME(4)(cnes_mmc_s* mmc, word addr)
{
    return 0;
}

void CNES_MAPPER_WRITE_FUNC_NAME(4)(cnes_mmc_s* mmc, word addr, byte value)
{
    switch (addr & 0xe001) {
        case 0x8000:
            mmc->control = value;
            break;
        case 0x8001: {
            uint reg = CONTROL_BANK_REGISTER(mmc->control);
            if (reg <= 5) {
                swap_chr_bank(mmc, 
                    reg, CONTROL_CHR_MODE(mmc->control), value);
            } else {    /* supposed to be 6, 7 */
                swap_prg_bank(mmc, 
                    reg, CONTROL_PRG_MODE(mmc->control), value & 0x3f);
            }
        }
        break;
        case 0xa000:
            if (mmc->ppu->mirroring != M_FOURSCREEN) {
                if (test_bit(value, 0)) {
                    mmc->ppu->mirroring = M_HORIZONAL;
                } else {
                    mmc->ppu->mirroring = M_VERTICAL;
                }
            }
            break;
        case 0xa001:
            if (test_bit(value, 6)) {
                // TODO: deny writes to PRG RAM
            } else {
                // TODO: allow writes to PRG RAM
            }
            if (test_bit(value, 7)) {
                // TODO: enable PRG RAM
            } else {
                // TODO: disable PRG RAM
            }
            break;
        case 0xc000:
            mmc->reload_value = value;
            break;
        case 0xc001:
            mmc->reload = TRUE;
            break;
        case 0xe000:
            mmc->irq_enabled = FALSE;
            // TODO: acknowledge any pending interrupts
            break;
        case 0xe001:
            mmc->irq_enabled = TRUE;
            break;
        default:
            assert(FALSE);
            break;
    }
}

void cnes_mmc3_update(cnes_mmc_s* mmc)
{
    if (mmc->rom->mapper != 4) {
        return;
    }
    if (mmc->reload) {
        mmc->counter = mmc->reload_value;
        mmc->reload = FALSE;
    }
    if (mmc->counter > 0) {
        mmc->counter --;
    } else {
        if (mmc->irq_enabled) {
            cnes_cpu_signal(mmc->cpu, INT_IRQ);
        }
        mmc->counter = mmc->reload_value;
    }
}