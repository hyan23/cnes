// rom.c
// Author: hyan23
// Date: 2017.08.01

#include "header.h"
#include "cartridge.h"
#include "rom.h"

static error_t read_header(cnes_header_s* header, cnes_cartridge_s* cartridge)
{
    fseek(cartridge->fin, 0, SEEK_SET);
    if (fread(header, sizeof (cnes_header_s), 1, cartridge->fin) != 1) {
        return FAILURE;
    }
    return SUCCESS;
}

static void dump_header(const cnes_header_s* header)
{
    printf("Magic: %c %c %c %02x\n", 
        header->magic[0], header->magic[1], header->magic[2], 
        header->magic[3]);
    printf("16K PRG-ROM page count: %u\n", header->prg_pgs);
    printf("8K CHR-ROM page count: %u\n", header->chr_pgs);
    printf("ROM Control Byte #1: 0x%02x\n", header->control1);
    printf("ROM Control Byte #2: 0x%02x\n", header->control2);
}

static cnes_mirroring_t mirroring_mode(const cnes_header_s* header)
{
    // mmc sets M_SINGLE
    if (test_bit(header->control1, 3)) {
        return M_FOURSCREEN;
    }
    if (!test_bit(header->control1, 0)) {
        return M_HORIZONAL;
    } else {
        return M_VERTICAL;
    }
}

static byte mapper_number(const cnes_header_s* header)
{
    return (header->control2 & 0xf0) | ((header->control1 & 0xf0) >> 4);
}

#define read_chr read_prg

static error_t read_prg(cnes_prg_s* prg, cnes_cartridge_s* cartridge)
{
    if (prg->size == 0) {   /* CHR-ROM page count can be zero */
        return SUCCESS;
    }
    fseek(cartridge->fin, prg->offset, SEEK_SET);
    if (fread(prg->rom, prg->size, 1, cartridge->fin) != 1) {
        return FAILURE;
    }
    return SUCCESS;
}

#define dump_chr dump_prg
static void dump_prg(const cnes_prg_s* prg)
{
    printf("Offset: 0x%08x\n", prg->offset);
    printf("Size: %u\n", prg->size);
    printf("Starting 64 Bytes:\n");
    dump(0, 16, prg->rom, 64);
}

error_t cnes_rom_open(cnes_rom_s* rom, cnes_cartridge_s* cartridge)
{
    CLEAR(rom);
    if (FAILED(read_header(&rom->header, cartridge))) {
        return READ_FILE_ERROR;
    }
    
    rom->mirroring = mirroring_mode(&rom->header);
    rom->mapper = mapper_number(&rom->header);
    rom->sram_present = test_bit(rom->header.control1, 1);
    BOOL trainer_present = test_bit(rom->header.control1, 2);
    
    rom->prg.offset = sizeof (cnes_header_s) + (trainer_present ? 512 : 0);
    rom->prg.size = rom->header.prg_pgs * PRG_PAGE_SIZE;
    assert(rom->header.prg_pgs > 0);
    rom->prg.rom = MALLOC(byte, rom->prg.size);
    if (NUL(rom->prg.rom)) {
        cnes_rom_close(rom);
        return MEMORY_OVERFLOW;
    }
    if (FAILED(read_prg(&rom->prg, cartridge))) {
        cnes_rom_close(rom);
        return READ_FILE_ERROR;
    }
    
    rom->chr.offset = rom->prg.offset + rom->prg.size;
    rom->chr.size = rom->header.chr_pgs * CHR_PAGE_SIZE;
    if (rom->header.chr_pgs > 0) {
        rom->chr.rom = MALLOC(byte, rom->prg.size);
    } else {
        rom->chr.rom = MALLOC(byte, CHR_PAGE_SIZE);
    }
    if (NUL(rom->chr.rom)) {
        cnes_rom_close(rom);
        return MEMORY_OVERFLOW;
    }
    if (rom->header.chr_pgs > 0) {
        if (FAILED(read_chr(&rom->chr, cartridge))) {
            cnes_rom_close(rom);
            return READ_FILE_ERROR;
        }
    } else {
        memset(rom->chr.rom, 0, CHR_PAGE_SIZE);
    }
    return SUCCESS;
}

void cnes_rom_close(cnes_rom_s* rom)
{
    FREE(rom->prg.rom);
    FREE(rom->chr.rom);
}

const char* MIRRORING_STR[4] = {
    "M_HORIZONAL", "M_VERTICAL", "M_SINGLE", "M_FOURSCREEN"
};

void cnes_rom_dump(const cnes_rom_s* rom)
{
    printf("ROM Header:\n");
    dump_header(&rom->header);
    printf("Mirroring: %s\n", MIRRORING_STR[rom->mirroring]);
    printf("Mapper: %u\n", rom->mapper);
    printf("SRAM Present: %s\n", rom->sram_present ? "True" : "False");
    printf("PRG ROM:\n");
    dump_prg(&rom->prg);
    printf("CHR ROM:\n");
    dump_chr(&rom->chr);
}