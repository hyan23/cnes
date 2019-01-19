// mapper9.c
// Author: hyan23
// Date: 2018.05.22

#include "mmc.h"

#define latch0      r0
#define latch1      r1
// chr bank select
// swap 8K CHR bank to ppu 0x0 when latch0 = 0xfd
#define chrfd0      r2
#define chrfe0      r3
// swap 8K CHR bank to ppu 0x1000 when latch1 = 0xfd
#define chrfd1      r4
#define chrfe1      r5

static void swap_chr_bank0(cnes_mmc_s* mmc)
{
    switch (mmc->latch0) {
        case 0xfd: {
            mmc_fourKbank_t src = MMC_GET_4KCHR_BANK(mmc, mmc->chrfd0);
            MMC_SWITCH_4K_BANK(mmc->layoutp->pattern0, src);
        }
        break;
        case 0xfe: {
            mmc_fourKbank_t src = MMC_GET_4KCHR_BANK(mmc, mmc->chrfe0);
            MMC_SWITCH_4K_BANK(mmc->layoutp->pattern0, src);
        }
        break;
        default:
            break;
    }
}

static void swap_chr_bank1(cnes_mmc_s* mmc)
{
    switch (mmc->latch1) {
        case 0xfd: {
            mmc_fourKbank_t src = MMC_GET_4KCHR_BANK(mmc, mmc->chrfd1);
            MMC_SWITCH_4K_BANK(mmc->layoutp->pattern1, src);
        }
        break;
        case 0xfe: {
            mmc_fourKbank_t src = MMC_GET_4KCHR_BANK(mmc, mmc->chrfe1);
            MMC_SWITCH_4K_BANK(mmc->layoutp->pattern1, src);
        }
        break;
        default:
            break;
    }
}

void CNES_MAPPER_INIT_FUNC_NAME(9)(cnes_mmc_s* mmc)
{
    mmc->name = "MMC2";
    mmc_eightKbank_t src = MMC_GET_8KPRG_BANK(mmc, MMC_8KPRG_BANK_NUMBER(mmc) - 3);
    MMC_SWITCH_8K_BANK(RAW_PTR(mmc->layout->lower) + 0x2000, src);
    src = MMC_GET_8KPRG_BANK(mmc, MMC_8KPRG_BANK_NUMBER(mmc) - 2);
    MMC_SWITCH_8K_BANK(mmc->layout->upper, src);
    src = MMC_GET_8KPRG_BANK(mmc, MMC_8KPRG_BANK_NUMBER(mmc) - 1);
    MMC_SWITCH_8K_BANK(RAW_PTR(mmc->layout->upper) + 0x2000, src);
    // TODO: CHR banks
}

byte CNES_MAPPER_READ_FUNC_NAME(9)(cnes_mmc_s* mmc, word addr)
{
    return 0;
}

void CNES_MAPPER_WRITE_FUNC_NAME(9)(cnes_mmc_s* mmc, word addr, byte value)
{
    switch (addr & 0xf000) {
        case 0xa000: {
            mmc_eightKbank_t src = MMC_GET_8KPRG_BANK(mmc, value & 0xf);
            MMC_SWITCH_8K_BANK(mmc->layout->lower, src);
        }
        break;
        case 0xb000:
            mmc->chrfd0 = value & 0x1f;
            break;
        case 0xc000:
            mmc->chrfe0 = value & 0x1f;
            break;
        case 0xd000:
            mmc->chrfd1 = value & 0x1f;
            break;
        case 0xe000:
            mmc->chrfe1 = value & 0x1f;
            break;
        case 0xf000:
            mmc->ppu->mirroring = test_bit(value, 0) ? 
                M_HORIZONAL : M_VERTICAL;
            break;
        default:
            assert(FALSE);
            break;
    }
}

void cnes_mmc2_update(cnes_mmc_s* mmc, word ppuaddr)
{
    if (mmc->rom->mapper != 9) {
        return;
    }
    switch (ppuaddr) {
        case 0xfd8:
            mmc->latch0 = 0xfd;
            swap_chr_bank0(mmc);
            break;
        case 0xfe8:
            mmc->latch0 = 0xfe;
            swap_chr_bank0(mmc);
            break;
        case 0x1fd8:
        case 0x1fd9:
        case 0x1fda:
        case 0x1fdb:
        case 0x1fdc:
        case 0x1fdd:
        case 0x1fde:
        case 0x1fdf:
            mmc->latch1 = 0xfd;
            swap_chr_bank1(mmc);
            break;
        case 0x1fe8:
        case 0x1fe9:
        case 0x1fea:
        case 0x1feb:
        case 0x1fec:
        case 0x1fed:
        case 0x1fee:
        case 0x1fef:
            mmc->latch1 = 0xfe;
            swap_chr_bank1(mmc);
            break;
        default:
            break;
    }
}