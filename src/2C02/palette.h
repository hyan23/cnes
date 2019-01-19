// palette.h
// Author: hyan23
// Date: 2018.05.20

#ifndef __PALETTE_H__
#define __PALETTE_H__

#include <SDL/SDL.h>

// 0x40 colors actually stored
#define CNES_PALETTE_COLORS     0x100

extern SDL_Color cnes_palette1[CNES_PALETTE_COLORS];
extern SDL_Color cnes_palette2[CNES_PALETTE_COLORS];

#endif /* __PALETTE_H__ */