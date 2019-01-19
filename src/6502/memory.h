// memory.h
// Author: hyan23
// Date: 2017.08.21

#ifndef __MEMORY_H__
#define __MEMORY_H__

#include "header.h"

struct CNES_MEMORY;

typedef byte (*cnes_memory_readbyte_func)(struct CNES_MEMORY* memory, word addr);
typedef word (*cnes_memory_readword_func)(struct CNES_MEMORY* memory, word addr);
typedef void (*cnes_memory_writebyte_func)(struct CNES_MEMORY* memory, 
    word addr, byte value);
typedef void (*cnes_memory_writeword_func)(struct CNES_MEMORY* memory, 
    word addr, word value);

typedef struct CNES_MEMORY
{
    cnes_memory_readbyte_func   readbyte;
    cnes_memory_readword_func   readword;
    cnes_memory_writebyte_func  writebyte;
    cnes_memory_writeword_func  writeword;
    void*   layout;
    void*   ref1;
    void*   ref2;
    void*   ref3;
    void*   ref4;
}
cnes_memory_s;

// flat mode
extern byte cnes_memory_readbyte(cnes_memory_s* memory, word addr);
extern word cnes_memory_readword(cnes_memory_s* memory, word addr);
extern void cnes_memory_writebyte(cnes_memory_s* memory, word addr, byte value);
extern void cnes_memory_writeword(cnes_memory_s* memory, word addr, word value);

#endif /* __MEMORY_H__ */