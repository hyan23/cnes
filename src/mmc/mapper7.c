// mapper7.c
// Author: hyan23
// Date: 2018.05.24

#include "mmc.h"

void CNES_MAPPER_INIT_FUNC_NAME(7)(cnes_mmc_s* mmc)
{
    mmc->name = "AxROM";
    mmc_thirtytwoKbank_t src = MMC_GET_32KPRG_BANK(mmc, 0);
    MMC_SWITCH_32K_BANK(mmc->layout->lower, src);
    if (MMC_8KCHR_BANK_NUMBER(mmc) > 0) {
        mmc_eightKbank_t chr = MMC_GET_8KCHR_BANK(mmc, 0);
        MMC_SWITCH_8K_BANK(mmc->layoutp->pattern0, chr);
    }
}

byte CNES_MAPPER_READ_FUNC_NAME(7)(cnes_mmc_s* mmc, word addr)
{
    return 0;
}

void CNES_MAPPER_WRITE_FUNC_NAME(7)(cnes_mmc_s* mmc, word addr, byte value)
{
    uint bank = value & 0x7;
    mmc_thirtytwoKbank_t src = MMC_GET_32KPRG_BANK(mmc, bank);
    MMC_SWITCH_32K_BANK(mmc->layout->lower, src);
    mmc->ppu->mirroring = test_bit(value, 4) ? M_SINGLEU : M_SINGLEL;
}