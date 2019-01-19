// mmc.c
// Author: hyan23
// Date: 2018.05.03

#include "mmc.h"

byte* mmc_get_prg_bank(struct CNES_MMC* mmc, 
uint banksize, uint banknum, uint bankno)
{
    assert(banknum > 0);
    assert(bankno < banknum);
    return RAW_PTR(mmc->rom->prg.rom) + bankno * banksize;
}

byte* mmc_get_chr_bank(struct CNES_MMC* mmc, 
uint banksize, uint banknum, uint bankno)
{
    assert(banknum > 0);
    assert(bankno < banknum);
    return RAW_PTR(mmc->rom->chr.rom) + bankno * banksize;
}

#define INIT    CNES_MAPPER_INIT_FUNC_NAME
#define READ    CNES_MAPPER_READ_FUNC_NAME
#define WRITE   CNES_MAPPER_WRITE_FUNC_NAME

#define MAPPER_INIT_FUNC_DECL(mapper)   \
    extern void INIT(mapper)(cnes_mmc_s* mmc)
#define MAPPER_READ_FUNC_DECL(mapper)   \
    extern byte READ(mapper)(cnes_mmc_s* mmc, word addr)
#define MAPPER_WRITE_FUNC_DECL(mapper)  \
    extern void WRITE(mapper)(cnes_mmc_s* mmc,  \
        word addr, byte value)

#define MAPPER_FUNC_DECL(mapper)        \
    MAPPER_INIT_FUNC_DECL(mapper);      \
    MAPPER_READ_FUNC_DECL(mapper);      \
    MAPPER_WRITE_FUNC_DECL(mapper)

static cnes_mapper_init_func    mapper_init[CNES_MAPPER_NUMBER];
static cnes_mapper_read_func    mapper_read[CNES_MAPPER_NUMBER];
static cnes_mapper_write_func   mapper_write[CNES_MAPPER_NUMBER];

#define MAPPER_FUNC_REGISTER(mapper)        \
    assert(mapper < CNES_MAPPER_NUMBER);    \
    MAPPER_FUNC_DECL(mapper);               \
    mapper_init[mapper] = INIT(mapper);     \
    mapper_read[mapper] = READ(mapper);     \
    mapper_write[mapper] = WRITE(mapper)

static void mapper_func_register(void)
{
/* Begin Mapper Registration */
    MAPPER_FUNC_REGISTER(0);
    MAPPER_FUNC_REGISTER(1);
    MAPPER_FUNC_REGISTER(2);
    MAPPER_FUNC_REGISTER(3);
    MAPPER_FUNC_REGISTER(4);
    MAPPER_FUNC_REGISTER(7);
    MAPPER_FUNC_REGISTER(9);
    MAPPER_FUNC_REGISTER(10);
/* End Mapper Registration */
}

void 
cnes_mmc_init(cnes_mmc_s* mmc, 
cnes_rom_s* rom, cnes_layout_s* layout, cnes_layoutp_s* layoutp, 
cnes_cpu_s* cpu, cnes_ppu_s* ppu) 
{
    CLEAR(mmc);
    mapper_func_register();
    mmc->rom = rom;
    mmc->layout = layout;
    mmc->layoutp = layoutp;
    mmc->cpu = cpu;
    mmc->ppu = ppu;
    mmc->init = mapper_init[mmc->rom->mapper];
    mmc->read = mapper_read[mmc->rom->mapper];
    mmc->write = mapper_write[mmc->rom->mapper];
    assert(NOTNULL(mmc->init) && NOTNULL(mmc->read) && NOTNULL(mmc->write));
    mmc->init(mmc);
}

#ifndef CNES_MMC_USE_MACRO_FUNC

byte cnes_mmc_read(cnes_mmc_s* mmc, word addr)
{
    return mmc->read(mmc, addr);
}

void cnes_mmc_write(cnes_mmc_s* mmc, word addr, byte value)
{
    mmc->write(mmc, addr, value);
}

#endif /* CNES_MMC_USE_MACRO_FUNC */


void cnes_mmc_dumpb(FILE* fout, const cnes_mmc_s* mmc)
{
    fwrite(&mmc->r0, 1, 1, fout);
    fwrite(&mmc->r1, 1, 1, fout);
    fwrite(&mmc->r2, 1, 1, fout);
    fwrite(&mmc->r3, 1, 1, fout);
    fwrite(&mmc->r4, 1, 1, fout);
    fwrite(&mmc->r5, 1, 1, fout);
    fwrite(&mmc->r6, 1, 1, fout);
    fwrite(&mmc->r7, 1, 1, fout);
}

void cnes_mmc_loadb(cnes_mmc_s* mmc, FILE* fin)
{
    fread(&mmc->r0, 1, 1, fin);
    fread(&mmc->r1, 1, 1, fin);
    fread(&mmc->r2, 1, 1, fin);
    fread(&mmc->r3, 1, 1, fin);
    fread(&mmc->r4, 1, 1, fin);
    fread(&mmc->r5, 1, 1, fin);
    fread(&mmc->r6, 1, 1, fin);
    fread(&mmc->r7, 1, 1, fin);
}