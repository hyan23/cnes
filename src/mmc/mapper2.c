// mapper2.c
// Author: hyan23
// Date: 2018.05.03

#include "mmc.h"

void CNES_MAPPER_INIT_FUNC_NAME(2)(cnes_mmc_s* mmc)
{
    mmc->name = "UxROM";
    mmc_sixteenKbank_t src = MMC_GET_16KPRG_BANK(mmc, 
        mmc->rom->header.prg_pgs - 1);
    MMC_SWITCH_16K_BANK(mmc->layout->upper, src);
    if (mmc->rom->header.chr_pgs > 0) {
        mmc_eightKbank_t chr = MMC_GET_8KCHR_BANK(mmc, 0);
        MMC_SWITCH_8K_BANK(mmc->layoutp->pattern0, chr);
    }
}

byte CNES_MAPPER_READ_FUNC_NAME(2)(cnes_mmc_s* mmc, word addr)
{
    return 0;
}

void CNES_MAPPER_WRITE_FUNC_NAME(2)(cnes_mmc_s* mmc, word addr, byte value)
{
    byte bank = value & 0x7;
    mmc_sixteenKbank_t src = MMC_GET_16KPRG_BANK(mmc, bank);
    MMC_SWITCH_16K_BANK(mmc->layout->lower, src);
}