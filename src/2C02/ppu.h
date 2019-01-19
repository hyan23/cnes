// ppu.h
// Author: hyan23
// Date: 2018.04.23

#ifndef __PPU_H__
#define __PPU_H__

#include "header.h"
#include "../6502/memory.h"
#include "../rom/rom.h"

// Sprite Temporary Memory
typedef struct CNES_PPU_STM
{
    struct STM_SPRITE
    {
        byte tile_index;
        byte xc;
        byte attr;      /* palette select, 2 bits */
        BOOL priority;  /* < playfield's on set */
        BOOL hf;        /* horizontal flip */
        byte cmp_result;    /* vertical flip is not included since logical 
                    inversion is applied to cmp_result if it is set */
    } sprite[8];
    byte    in_range;   /* 4 bits, in range counter */
    BOOL    found;      /* primary sprite(#0) was found in range */
}
cnes_ppu_stm_s;

// Sprite Buffer Memory
typedef struct CNES_PPU_SBM
{
    struct SBM_BUFFER 
    {
        byte    shift1;
        byte    shift2;
        byte    dc;         /* down counter */
        byte    attr;
        BOOL    priority;
        byte    shift;
    } buffer[8];
    BOOL    primary_present;
}
cnes_ppu_sbm_s;

/* Control Register #1 */
#define PPU_NMI_ENABLED(ppu)    test_bit((ppu).control_register1, 7)
#define PPU_SPR_SIZE(ppu)       test_bit((ppu).control_register1, 5)
#define PPU_BG_TILE_TBL(ppu)    (test_bit((ppu).control_register1, 4) ? 0x1000 : 0)
#define PPU_SPR_TILE_TBL(ppu)   (test_bit((ppu).control_register1, 3) ? 0x1000 : 0)
#define PPU_ADDR_INC(ppu)       (test_bit((ppu).control_register1, 2) ? 32 : 1)
#define PPU_NN(ppu)             ((ppu).control_register1 & 0x3)

/* Control Register #2 */
#define PPU_COLOR_EMPHASIS(ppu) ((ppu).control_register2 >> 5)
#define PPU_SPR_VISIBLE(ppu)    test_bit((ppu).control_register2, 4)
#define PPU_BG_VISIBLE(ppu)     test_bit((ppu).control_register2, 3)
#define PPU_SPR_CLIP(ppu)       (!test_bit((ppu).control_register2, 2))
#define PPU_BG_CLIP(ppu)        (!test_bit((ppu).control_register2, 1))
#define PPU_MONOCHROME(ppu)     test_bit((ppu).control_register2, 0)

/* PPU Status Register */
#define PPU_VBLANK_SET(ppu)     set_bit(&(ppu)->status_register, 7)
#define PPU_VBLANK_CLEAR(ppu)   reset_bit(&(ppu)->status_register, 7)
#define PPU_VBLANK(ppu)         test_bit((ppu).status_register, 7)
#define PPU_HIT_SET(ppu)        set_bit(&(ppu)->status_register, 6)
#define PPU_HIT_CLEAR(ppu)      reset_bit(&(ppu)->status_register, 6)
#define PPU_OVERFLOW_SET(ppu)   set_bit(&(ppu)->status_register, 5)
#define PPU_OVERFLOW_CLEAR(ppu) reset_bit(&(ppu)->status_register, 5)
#define PPU_STATUS_CLEAR(ppu)   ((ppu)->status_register &= 0x1f)

/* PPU Address */
#define PPU_TEMP_SET_NN(temp, NN)       (*(temp) = (*(temp) & 0xf3ff) | \
                                (((NN) & 0x3) << 10))
#define PPU_TEMP_GET_NN(temp)           (((temp) >> 10) & 0x3)
#define PPU_TEMP_FLIP_NNL(temp)         (*(temp) ^= 0x400)
#define PPU_TEMP_FLIP_NNH(temp)         (*(temp) ^= 0x800)
#define PPU_TOGGLE_SET(toggle)          (*(toggle) = TRUE)
#define PPU_TOGGLE_CLEAR(toggle)        (*(toggle) = FALSE)
#define PPU_TEMP_SET_TILEX(temp, value) (*(temp) = (*(temp) & 0x7fe0) | \
                                ((value) & 0x1f))
#define PPU_TEMP_GET_TILEX(temp)        ((temp) & 0x1f)
#define PPU_SET_FINEX(ppu, value)       ((ppu)->fineX = (value) & 0x7)
#define PPU_GET_FINEX(ppu)              ((ppu).fineX & 0x7)
#define PPU_TEMP_SET_TILEYL(temp, value)    (*(temp) = (*(temp) & 0x7f1f) | \
                                (((value) & 0x7) << 5))
#define PPU_TEMP_SET_TILEYH(temp, value)    (*(temp) = (*(temp) & 0x7cff) | \
                                (((value) & 0x3) << 8))
#define PPU_TEMP_SET_TILEY(temp, value) (*(temp) = (*(temp) & 0x7c1f) | \
                                (((value) & 0x1f) << 5))
#define PPU_TEMP_GET_TILEY(temp)        (((temp) >> 5) & 0x1f)
#define PPU_TEMP_SET_FINEY(temp, value) ((*temp) = (*(temp) & 0xfff) |  \
                                (((value) & 0x7) << 12))
#define PPU_TEMP_GET_FINEY(temp)        (((temp) >> 12) & 0x7)

/* Sprite RAM */
#define PPU_SPRITE_ROOMS            64
#define PPU_SPRITE_RAM              (PPU_SPRITE_ROOMS * 4)
#define PPU_SPRITES_SCANLINE        8

/* Sprite */
typedef byte (*sprite_t)[4];
#define PPU_GET_SPRITE(ppu, index)      (((sprite_t) (ppu).sprite_ram) + (index))
#define PPU_SPRITE_YC(sprite)           ((sprite)[0])
#define PPU_SPRITE_TILE_INDEX(sprite)   ((sprite)[1])
#define PPU_SPRITE_ATTR(sprite)         ((sprite)[2])
#define PPU_SPRITE_XC(sprite)           ((sprite)[3])
#define PPU_SPRITE_ATTR_VF(attr)        test_bit((attr), 7)
#define PPU_SPRITE_ATTR_HF(attr)        test_bit((attr), 6)
#define PPU_SPRITE_ATTR_PRIORITY(attr)  test_bit((attr), 5)
#define PPU_SPRITE_ATTR_VF(attr)        test_bit((attr), 7)
#define PPU_SPRITE_ATTR_PS(attr)        ((attr) & 0x3)

/* PPU Cycle */
#define PPU_SCANLINE_CYCLES         341
#define PPU_TRANSPARENT(pixel)      (((pixel) & 0x3) == 0)
#define PPU_ONE_VISIBLE(ppu)        (PPU_BG_VISIBLE(ppu) || PPU_SPR_VISIBLE(ppu))
#define PPU_BOTH_VISIBLE(ppu)       (PPU_BG_VISIBLE(ppu) && PPU_SPR_VISIBLE(ppu))

typedef struct CNES_PPU
{
    /* PPU */
    cnes_memory_s*  memory;
    cnes_mirroring_t    mirroring;
    byte    mapper;
    byte    sprite_ram[PPU_SPRITE_RAM];
    cnes_ppu_stm_s  stm;
    cnes_ppu_sbm_s  sbm;
    /* registers */
    byte    control_register1;      /* write only */
    byte    control_register2;      /* write only */
    byte    status_register;        /* read only */
    byte    vramal;     /* write only */
    byte    vramah;     /* write only */
    /* ppu addresses */
    word    temp;       /* 15 bits */
    BOOL    toggle;
    byte    fineX;
    word    address;
    byte    sprite_index;
    /* I/O */
    byte    read_cache;
    byte    value;
    byte    write_cache;        /* Not sure */
    /* rendering */
    BOOL    reload;     /* reload shift registers on next cycle */
    byte    b1;         /* fetched tile bitmap #1 */
    byte    b2;         /* fetched tile bitmap #2 */
    byte    attr;       /* pallete select */
    word    shift1;
    word    shift2;
    byte    shift3;
    byte    shift4;
    byte    latch;
    /* execute */
    uint    line;
    uint    scan_line;
    uint    pixel;
    uint    num;    /* number sprites to be drawn in next scan line */
    byte    image[240][256];
}
cnes_ppu_s;

extern void cnes_ppu_write(cnes_ppu_s* ppu, word addr, byte value);
extern byte cnes_ppu_read(cnes_ppu_s* ppu, word addr);
extern BOOL cnes_ppu_line(cnes_ppu_s* ppu);
extern void cnes_ppu_dumpb(FILE* fout, const cnes_ppu_s* ppu);
extern void cnes_ppu_loadb(cnes_ppu_s* ppu, FILE* fin);

#endif /* __PPU_H__ */