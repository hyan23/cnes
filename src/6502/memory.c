// memory.c
// Author: hyan23
// Date: 2018.05.18

#include "header.h"
#include "memory.h"

byte cnes_memory_readbyte(cnes_memory_s* memory, word addr)
{
    return ((byte*) memory->layout)[addr];
}

word cnes_memory_readword(cnes_memory_s* memory, word addr)
{
    return cnes_memory_readbyte(memory, addr + 1) << 8 |
        cnes_memory_readbyte(memory, addr); 
}

void cnes_memory_writebyte(cnes_memory_s* memory, word addr, byte value)
{
    ((byte*) memory->layout)[addr] = value;
}

void cnes_memory_writeword(cnes_memory_s* memory, word addr, word value)
{
    cnes_memory_writebyte(memory, addr, value & 0xff);
    cnes_memory_writebyte(memory, addr + 1, value >> 8);
}