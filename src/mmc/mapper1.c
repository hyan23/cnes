// mapper1.c
// Author: hyan23
// Date: 2018.05.03

#include "mmc.h"

// MMC1 internal registers
#define control         r0
#define chrbank0        r1
#define chrbank1        r2
#define prgbank         r3

#define CONTROL_INITIAL_VALUE   0xc

#define CONTROL_MIRRORING(control)  ((control) & 0x3)
#define CONTROL_PRG_MODE(control)   (((control) >> 2) & 0x3)
#define CONTROL_CHR_MODE(control)   (((control) >> 4) & 0x1)

#define PRGBANK_BANK(prgbank)       ((prgbank) & 0xf)
// PRG RAM Enabled
#define PRGBANK_ENABLED(prgbank)    (!test_bit((prgbank), 4))

// MMC1 I/O registers
#define shift_register  r4  /* 5 bits */
#define rselect         r5  /* register select, 2 bits */

#define SHIFT_INITIAL_VALUE     0x10

static void change_mirroring(cnes_mmc_s* mmc)
{
    const static cnes_mirroring_t mirrorings[4] = {
        M_SINGLEL,  M_SINGLEU,  M_VERTICAL, M_HORIZONAL
    };
    mmc->ppu->mirroring = mirrorings[CONTROL_MIRRORING(mmc->control)];
}

static void swap_fixed_bank(cnes_mmc_s* mmc)
{
    switch (CONTROL_PRG_MODE(mmc->control)) {
        case 0:
        case 1:
            break;
        case 2: {
            mmc_sixteenKbank_t src = MMC_GET_16KPRG_BANK(mmc, 0);
            MMC_SWITCH_16K_BANK(mmc->layout->lower, src);
        }
        break;
        case 3: {
            mmc_sixteenKbank_t src = MMC_GET_16KPRG_BANK(mmc, 
                mmc->rom->header.prg_pgs - 1);
            MMC_SWITCH_16K_BANK(mmc->layout->upper, src);
        }
        break;
        default:
            assert(FALSE);
            break;
    }
}

static void swap_prg_bank(cnes_mmc_s* mmc)
{
    switch (CONTROL_PRG_MODE(mmc->control)) {
        case 0:
        case 1: {
            mmc_thirtytwoKbank_t src = MMC_GET_32KPRG_BANK(mmc, 
                PRGBANK_BANK(mmc->prgbank) >> 1);
            MMC_SWITCH_32K_BANK(mmc->layout->lower, src);
        }
        break;
        case 2: {
            mmc_sixteenKbank_t src = MMC_GET_16KPRG_BANK(mmc, 
                PRGBANK_BANK(mmc->prgbank));
            MMC_SWITCH_16K_BANK(mmc->layout->upper, src);
        }
        break;
        case 3: {
            mmc_sixteenKbank_t src = MMC_GET_16KPRG_BANK(mmc, 
                PRGBANK_BANK(mmc->prgbank));
            MMC_SWITCH_16K_BANK(mmc->layout->lower, src);
        }
        break;
        default:
            assert(FALSE);
            break;
    }
}

static void swap_chr_bank(cnes_mmc_s* mmc)
{
    if (mmc->rom->header.chr_pgs == 0) {
        return;
    }
    switch (CONTROL_CHR_MODE(mmc->control)) {
        case 0: {
            mmc_eightKbank_t src = MMC_GET_8KCHR_BANK(mmc, mmc->chrbank0 >> 1);
            MMC_SWITCH_8K_BANK(mmc->layoutp->pattern0, src);
        }
        break;
        case 1: {
            mmc_fourKbank_t src = MMC_GET_4KCHR_BANK(mmc, mmc->chrbank0);
            MMC_SWITCH_4K_BANK(mmc->layoutp->pattern0, src);
            src = MMC_GET_4KCHR_BANK(mmc, mmc->chrbank1);
            MMC_SWITCH_4K_BANK(mmc->layoutp->pattern1, src);
        }
        break;
        default:
            assert(FALSE);
            break;
    }
}

void CNES_MAPPER_INIT_FUNC_NAME(1)(cnes_mmc_s* mmc)
{
    mmc->shift_register = SHIFT_INITIAL_VALUE;
    mmc->control = CONTROL_INITIAL_VALUE;
    change_mirroring(mmc);
    swap_fixed_bank(mmc);
    swap_prg_bank(mmc);
    swap_chr_bank(mmc);
    mmc->name = "MMC1";
}

byte CNES_MAPPER_READ_FUNC_NAME(1)(cnes_mmc_s* mmc, word addr)
{
    return 0;
}

void CNES_MAPPER_WRITE_FUNC_NAME(1)(cnes_mmc_s* mmc, word addr, byte value)
{
    if (test_bit(value, 7)) {
        mmc->shift_register = SHIFT_INITIAL_VALUE;
        mmc->control |= CONTROL_INITIAL_VALUE;
    } else {
        // shift register isn't full
        if (!test_bit(mmc->shift_register, 0)) {
            mmc->shift_register >>= 1;
            if (test_bit(value, 0)) {
                set_bit(&mmc->shift_register, 4);
            }
        } else {
            mmc->rselect = (addr >> 13) & 0x3;
            byte temp = (value & 0x1) << 4 | mmc->shift_register >> 1;
            switch (mmc->rselect) {
                case 0:
                    mmc->control = temp;
                    change_mirroring(mmc);
                    swap_fixed_bank(mmc);
                    swap_prg_bank(mmc);
                    swap_chr_bank(mmc);
                    break;
                case 1:
                    mmc->chrbank0 = temp;
                    swap_chr_bank(mmc);
                    break;
                case 2:
                    mmc->chrbank1 = temp;
                    swap_chr_bank(mmc);
                    break;
                case 3:
                    mmc->prgbank = temp;
                    swap_prg_bank(mmc);
                    break;
                default:
                    assert(FALSE);
                    break;
            }
            mmc->shift_register = SHIFT_INITIAL_VALUE;
        }
    }
}