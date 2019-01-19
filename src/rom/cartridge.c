// cartridge.c
// Author: hyan23
// Date: 2018.05.17

#include "header.h"
#include "common.h"
#include "cartridge.h"

error_t 
cnes_cartridge_open(cnes_cartridge_s* cartridge, 
const char* file)
{
    CLEAR(cartridge);
    cartridge->fin = fopen(file, "rb");
    if (NUL(cartridge->fin)) {
        return OPEN_FILE_ERROR;
    }
    
    char magic[4];
    CLEAR(magic);
    if (fread(magic, sizeof (magic), 1, cartridge->fin) != 1) {
        cnes_cartridge_close(cartridge);
        return READ_FILE_ERROR;
    }
    
    if (magic[0] != 'N' || magic[1] != 'E' || magic[2] != 'S' || 
        magic[3] != 0x1a) {
        cnes_cartridge_close(cartridge);
        return INVALID_FILE_ERROR;
    }
    
    fseek(cartridge->fin, 0, SEEK_END);
    cartridge->size = (uint) ftell(cartridge->fin);
    strcpy(cartridge->file, file);
    basename(cartridge->name, file);
    return SUCCESS;
}

void cnes_cartridge_close(cnes_cartridge_s* cartridge)
{
    if (NOTNULL(cartridge->fin)) {
        fclose(cartridge->fin);
    }
}

void cnes_cartridge_dump(const cnes_cartridge_s* cartridge)
{
    printf("File: %s\n", cartridge->file);
    printf("Size: %u\n", cartridge->size);
}