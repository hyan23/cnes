// mapper3.c
// Author: hyan23
// Date: 2018.05.03

#include "mmc.h"

extern void CNES_MAPPER_INIT_FUNC_NAME(0)(cnes_mmc_s* mmc);

void CNES_MAPPER_INIT_FUNC_NAME(3)(cnes_mmc_s* mmc)
{
    CNES_MAPPER_INIT_FUNC_NAME(0)(mmc);
    mmc->name = "CNROM";
}

byte CNES_MAPPER_READ_FUNC_NAME(3)(cnes_mmc_s* mmc, word addr)
{
    return 0;
}

void CNES_MAPPER_WRITE_FUNC_NAME(3)(cnes_mmc_s* mmc, word addr, byte value)
{
    byte bank = value & 0x3;
    mmc_eightKbank_t src = MMC_GET_8KCHR_BANK(mmc, bank);
    MMC_SWITCH_8K_BANK(mmc->layoutp->pattern0, src);
}