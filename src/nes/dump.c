// dump.c
// Author: hyan23
// Date: 2018.05.28

#include "header.h"
#include "../input/input.h"
#include "../2A03/layout.h"
#include "../6502/cpu.h"
#include "../2C02/layoutp.h"
#include "../2C02/ppu.h"
#include "../2A03/pAPU.h"
#include "nes.h"
#include "dump.h"

static void dump_file_location(const char* cartridge, char* buffer)
{
    strncat(buffer, cartridge, FILENAME);
    strncat(buffer, ".df", FILENAME);
}

error_t 
cnes_dump(const cnes_nes_s* nes, 
cnes_dump_t type, CNES_DUMP_ITEMS items)
{
    char filename[FILENAME];
    dump_file_location(nes->cartridge.file, filename);
    FILE* fout = fopen(filename, "w");
    if (NUL(fout)) {
        return OPEN_FILE_ERROR;
    }
    cnes_dump_header_s header;
    CLEAR(&header);
    strcpy(header.magic, "cnes");
    header.items = items;
    fwrite(&header, sizeof (header), 1, fout);
    if (test_bit(items, DUMP_INPUT)) {
        header.input_offset = ftell(fout);
        printf("input_offset: %d\n", header.input_offset);
        fwrite(&nes->input, sizeof (cnes_input_s), 1, fout);
    }
    if (test_bit(items, DUMP_LAYOUT)) {
        header.layout_offset = ftell(fout);
        printf("layout_offset: %d\n", header.layout_offset);
        fwrite(&nes->layout, sizeof (cnes_layout_s), 1, fout);
    }
    if (test_bit(items, DUMP_CPU)) {
        header.cpu_offset = ftell(fout);
        printf("cpu_offset: %d\n", header.cpu_offset);
        cnes_cpu_dumpb(fout, &nes->cpu);
    }
    if (test_bit(items, DUMP_LAYOUTP)) {
        header.layoutp_offset = ftell(fout);
        printf("layoutp_offset: %d\n", header.layoutp_offset);
        fwrite(&nes->layoutp, sizeof (cnes_layoutp_s), 1, fout);
    }
    if (test_bit(items, DUMP_PPU)) {
        header.ppu_offset = ftell(fout);
        printf("ppu_offset: %d\n", header.ppu_offset);
        cnes_ppu_dumpb(fout, &nes->ppu);
    }
    if (test_bit(items, DUMP_MMC)) {
        header.mmc_offset = ftell(fout);
        printf("mmc_offset: %d\n", header.mmc_offset);
        cnes_mmc_dumpb(fout, &nes->mmc);
    }
    if (test_bit(items, DUMP_APU)) {
        header.apu_offset = ftell(fout);
        printf("apu_offset: %d\n", header.apu_offset);
        cnes_pAPU_apu_dumpb(fout, &nes->apu);
    }
    fseek(fout, 0, SEEK_SET);
    fwrite(&header, sizeof (header), 1, fout);
    fclose(fout);
    printf("Dump to file: %s\n", filename);
    return SUCCESS;
}

error_t cnes_load(cnes_nes_s* nes, cnes_dump_t type)
{
    char filename[FILENAME];
    dump_file_location(nes->cartridge.file, filename);
    FILE* fin = fopen(filename, "r");
    if (NUL(fin)) {
        return OPEN_FILE_ERROR;
    }
    cnes_dump_header_s header;
    fread(&header, sizeof (header), 1, fin);
    if (!STREQU(header.magic, CNES_DUMP_MAGIC)) {
        fclose(fin);
        return INVALID_FILE_ERROR;
    }
    if (test_bit(header.items, DUMP_INPUT)) {
        fseek(fin, header.input_offset, SEEK_SET);
        fread(&nes->input, sizeof (cnes_input_s), 1, fin);
    }
    if (test_bit(header.items, DUMP_LAYOUT)) {
        fseek(fin, header.layout_offset, SEEK_SET);
        fread(&nes->layout, sizeof (cnes_layout_s), 1, fin);
    }
    if (test_bit(header.items, DUMP_CPU)) {
        fseek(fin, header.cpu_offset, SEEK_SET);
        cnes_cpu_loadb(&nes->cpu, fin);
    }
    if (test_bit(header.items, DUMP_LAYOUTP)) {
        fseek(fin, header.layoutp_offset, SEEK_SET);
        fread(&nes->layoutp, sizeof (cnes_layoutp_s), 1, fin);
    }
    if (test_bit(header.items, DUMP_PPU)) {
        fseek(fin, header.ppu_offset, SEEK_SET);
        cnes_ppu_loadb(&nes->ppu, fin);
    }
    if (test_bit(header.items, DUMP_MMC)) {
        fseek(fin, header.mmc_offset, SEEK_SET);
        cnes_mmc_loadb(&nes->mmc, fin);
    }
    if (test_bit(header.items, DUMP_APU)) {
        fseek(fin, header.apu_offset, SEEK_SET);
        cnes_pAPU_apu_loadb(&nes->apu, fin);
    }
    fclose(fin);
    printf("Load from file: %s\n", filename);
    return SUCCESS;
}