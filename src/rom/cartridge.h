// cartridge.h
// Author: hyan23
// Date: 2018.05.17

#ifndef __CARTRIDGE_H__
#define __CARTRIDGE_H__

#include "header.h"

typedef struct CNES_CARTRIDGE
{
    char    file[FILENAME];
    char    name[FILENAME];
    uint    size;
    FILE*   fin;
}
cnes_cartridge_s;

extern error_t cnes_cartridge_open(cnes_cartridge_s* cartridge, 
    const char* file);
extern void cnes_cartridge_close(cnes_cartridge_s* cartridge);
extern void cnes_cartridge_dump(const cnes_cartridge_s* cartridge);

#endif /* __CARTRIDGE_H__ */
