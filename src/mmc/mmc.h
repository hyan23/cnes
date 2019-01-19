// mmc.h
// Author: hyan23
// Date: 2018.05.03

#ifndef __MMC_H__
#define __MMC_H__

#include "header.h"
#include "../rom/rom.h"
#include "../2A03/layout.h"
#include "../6502/cpu.h"
#include "../2C02/layoutp.h"
#include "../2C02/ppu.h"

#define CNES_MMC_USE_MACRO_FUNC

#define CNES_MAPPER_NUMBER      256

#define CNES_MAPPER_FUNC_NAME(mapper, func) \
    cnes_mapper##mapper##func
#define CNES_MAPPER_INIT_FUNC_NAME(mapper)  \
    CNES_MAPPER_FUNC_NAME(mapper, _init)
#define CNES_MAPPER_READ_FUNC_NAME(mapper)  \
    CNES_MAPPER_FUNC_NAME(mapper, _read)
#define CNES_MAPPER_WRITE_FUNC_NAME(mapper) \
    CNES_MAPPER_FUNC_NAME(mapper, _write)

struct CNES_MMC;

typedef void (*cnes_mapper_init_func)(struct CNES_MMC* mmc);
typedef byte (*cnes_mapper_read_func)(struct CNES_MMC* mmc, word addr);
typedef void (*cnes_mapper_write_func)(struct CNES_MMC* mmc, 
    word addr, byte value);

// internal uses
extern byte* mmc_get_prg_bank(struct CNES_MMC* mmc, 
    uint banksize, uint banknum, uint bankno);
extern byte* mmc_get_chr_bank(struct CNES_MMC* mmc, 
    uint banksize, uint banknum, uint bankno);

#define MMC_PRG_BANK_NUMBER(mmc, type)      \
    ((mmc)->rom->header.prg_pgs * PRG_PAGE_SIZE /   \
    sizeof (*((type) NULL)))
#define MMC_CHR_BANK_NUMBER(mmc, type)      \
    ((mmc)->rom->header.chr_pgs * CHR_PAGE_SIZE /   \
    sizeof (*((type) NULL)))
#define MMC_PRG_BANK_MASK(mmc, type)        \
    (MMC_PRG_BANK_NUMBER((mmc), type) - 1)
#define MMC_CHR_BANK_MASK(mmc, type)        \
    (MMC_CHR_BANK_NUMBER((mmc), type) - 1)
#define MMC_GET_PRG_BANK(mmc, type, bank)   \
    ((type) mmc_get_prg_bank((mmc),     \
    sizeof (*((type) NULL)), MMC_PRG_BANK_NUMBER((mmc), type),  \
    (bank) & MMC_PRG_BANK_MASK((mmc), type)))   /* Not sure */
#define MMC_GET_CHR_BANK(mmc, type, bank)   \
    ((type) mmc_get_chr_bank((mmc),     \
    sizeof (*((type) NULL)), MMC_CHR_BANK_NUMBER((mmc), type),  \
    (bank) & MMC_CHR_BANK_MASK((mmc), type)))   /* Not sure */
#define MMC_SWITCH_BANK(dest, src, type)    \
    memcpy((dest), (src), sizeof (*((type) NULL)))

typedef byte (*mmc_thirtytwoKbank_t)[0x8000];
#define MMC_GET_32KPRG_BANK(mmc, bank)  \
    MMC_GET_PRG_BANK((mmc), mmc_thirtytwoKbank_t, (bank))
#define MMC_SWITCH_32K_BANK(dest, src)  \
    MMC_SWITCH_BANK((dest), (src), mmc_thirtytwoKbank_t)
    
typedef byte (*mmc_sixteenKbank_t)[0x4000];
#define MMC_GET_16KPRG_BANK(mmc, bank)  \
    MMC_GET_PRG_BANK((mmc), mmc_sixteenKbank_t, (bank))
#define MMC_SWITCH_16K_BANK(dest, src)  \
    MMC_SWITCH_BANK((dest), (src), mmc_sixteenKbank_t)

typedef byte (*mmc_eightKbank_t)[0x2000];
#define MMC_8KPRG_BANK_NUMBER(mmc)      \
    MMC_PRG_BANK_NUMBER((mmc), mmc_eightKbank_t)
#define MMC_8KCHR_BANK_NUMBER(mmc)      \
    MMC_CHR_BANK_NUMBER((mmc), mmc_eightKbank_t)
#define MMC_GET_8KPRG_BANK(mmc, bank)   \
    MMC_GET_PRG_BANK((mmc), mmc_eightKbank_t, (bank))
#define MMC_GET_8KCHR_BANK(mmc, bank)   \
    MMC_GET_CHR_BANK((mmc), mmc_eightKbank_t, (bank))
#define MMC_SWITCH_8K_BANK(dest, src)   \
    MMC_SWITCH_BANK((dest), (src), mmc_eightKbank_t)

typedef byte (*mmc_fourKbank_t)[0x1000];
#define MMC_GET_4KCHR_BANK(mmc, bank)   \
    MMC_GET_CHR_BANK((mmc), mmc_fourKbank_t, (bank))
#define MMC_SWITCH_4K_BANK(dest, src)   \
    MMC_SWITCH_BANK((dest), (src), mmc_fourKbank_t)
    
typedef byte (*mmc_oneKbank_t)[0x400];
#define MMC_GET_1KCHR_BANK(mmc, bank)   \
    MMC_GET_CHR_BANK((mmc), mmc_oneKbank_t, (bank))
#define MMC_SWITCH_1K_BANK(dest, src)   \
    MMC_SWITCH_BANK((dest), (src), mmc_oneKbank_t)

typedef struct CNES_MMC
{
    cnes_rom_s*     rom;
    cnes_layout_s*  layout;
    cnes_layoutp_s* layoutp;
    cnes_cpu_s*     cpu;
    cnes_ppu_s*     ppu;
    cnes_mapper_init_func   init;
    cnes_mapper_read_func   read;
    cnes_mapper_write_func  write;
    const char* name;
    byte    r0;     /* mmc common registers */
    byte    r1;
    byte    r2;
    byte    r3;
    byte    r4;
    byte    r5;
    byte    r6;
    byte    r7;
}
cnes_mmc_s;

extern void cnes_mmc_init(cnes_mmc_s* mmc, 
    cnes_rom_s* rom, cnes_layout_s* layout, cnes_layoutp_s* layoutp, 
    cnes_cpu_s* cpu, cnes_ppu_s* ppu);
#ifndef CNES_MMC_USE_MACRO_FUNC
    extern byte cnes_mmc_read(cnes_mmc_s* mmc, word addr);
    extern void cnes_mmc_write(cnes_mmc_s* mmc, word addr, byte value);
#else
    #define cnes_mmc_read(mmc, addr)            (mmc)->read((mmc), (addr))
    #define cnes_mmc_write(mmc, addr, value)    (mmc)->write((mmc), (addr), (value))
#endif /* CNES_MMC_USE_MACRO_FUNC */
extern void cnes_mmc_dumpb(FILE* fout, const cnes_mmc_s* mmc);
extern void cnes_mmc_loadb(cnes_mmc_s* mmc, FILE* fin);

extern void cnes_mmc3_update(cnes_mmc_s* mmc);
extern void cnes_mmc2_update(cnes_mmc_s* mmc, word ppuaddr);
extern void cnes_mmc4_update(cnes_mmc_s* mmc, word ppuaddr);

#endif /* __MMC_H__ */