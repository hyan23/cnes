// 2A03.h
// Author: hyan23
// Date: 2017.08.20

#ifndef __2A03_H__
#define __2A03_H__

#define CNES_2A03_ZERO_PAGE     0x0
#define CNES_2A03_STACK         0x100
#define CNES_2A03_RAM           0x200
#define CNES_2A03_REGISTER1     0x2000
#define CNES_2A03_REGISTER2     0x4000
#define CNES_2A03_EXPANSION     0x4020
#define CNES_2A03_SRAM          0x6000
#define CNES_2A03_LOWER_BANK    0x8000
#define CNES_2A03_UPPER_BANK    0xc000

#define CNES_2A03_PPUW1         0x2000
#define CNES_2A03_PPUW2         0x2001
#define CNES_2A03_PPUR1         0x2002
#define CNES_2A03_SPRA          0x2003
#define CNES_2A03_SPRD          0x2004
#define CNES_2A03_VRAMAL        0x2005
#define CNES_2A03_VRAMAH        0x2006
#define CNES_2A03_VRAMD         0x2007

#define CNES_2A03_APU_P1C       0x4000
#define CNES_2A03_APU_P1RC      0x4001
#define CNES_2A03_APU_P1FT      0x4002
#define CNES_2A03_APU_P1CT      0x4003
#define CNES_2A03_APU_P2C       0x4004
#define CNES_2A03_APU_P2RC      0x4005
#define CNES_2A03_APU_P2FT      0x4006
#define CNES_2A03_APU_P2CT      0x4007
#define CNES_2A03_APU_TRIC1     0x4008
#define CNES_2A03_APU_TRIF1     0x400a
#define CNES_2A03_APU_TRIF2     0x400b
#define CNES_2A03_APU_NOIC1     0x400c
#define CNES_2A03_APU_NOIF1     0x400e
#define CNES_2A03_APU_NOIF2     0x400f
#define CNES_2A03_APU_DMC       0x4010
#define CNES_2A03_APU_DMDA      0x4011
#define CNES_2A03_APU_DMA       0x4012      /* Delta Modulation Channel Address */
#define CNES_2A03_APU_MODDL     0x4013
#define CNES_2A03_APU_SIG       0x4015

#define CNES_2A03_SPR_DMA       0x4014      /* Direct Memory Access */

#define CNES_2A03_PORT1         0x4016
#define CNES_2A03_PORT2         0x4017

#endif /* __2A03_H__ */