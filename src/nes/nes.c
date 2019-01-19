// nes.c
// Author: hyan23
// Date: 2018.05.28

#include <stdio.h>
#include <stdlib.h> /* system */
#include "header.h"
#include "common.h"
#include "../rom/cartridge.h"
#include "../rom/rom.h"
#include "../mmc/mmc.h"
#include "../input/connector.h"
#include "../input/input.h"
#include "../6502/memory.h"
#include "../6502/cpu.h"
#include "../2A03/layout.h"
#include "../2C02/layoutp.h"
#include "../2C02/video.h"
#include "../2C02/timer.h"
#include "../2C02/ppu.h"
#include "../2A03/pAPU.h"
#include "../ini/ini.h"
#include "dump.h"
#include "nes.h"

error_t cnes_nes_init(cnes_nes_s* nes, const cnes_conf_s* conf)
{
    CLEAR(nes);
    memcpy(&nes->conf, conf, sizeof (cnes_conf_s));
    error_t result = cnes_cartridge_open(&nes->cartridge, nes->conf.emulator.file);
    if (FAILED(result)) {
        return result;
    }
    printf("Open ROM File: \n");
    cnes_cartridge_dump(&nes->cartridge);
    
    result = cnes_rom_open(&nes->rom, &nes->cartridge);
    if (FAILED(result)) {
        cnes_cartridge_close(&nes->cartridge);
        return result;
    }
    printf("Open Rom: \n");
    cnes_rom_dump(&nes->rom);
    
    cnes_input_init(&nes->input);
    cnes_connector_init(&nes->connector);
    cnes_connector_dump(&nes->connector);
    
    CLEAR(&nes->layout);
    CLEAR(&nes->memory);
    nes->memory.layout = &nes->layout;
    nes->memory.readbyte = cnes_layout_readbyte;
    nes->memory.readword = cnes_layout_readword;
    nes->memory.writebyte = cnes_layout_writebyte;
    nes->memory.writeword = cnes_layout_writeword;
    CNES_MEMORY_MMC(nes->memory) = &nes->mmc;
    CNES_MEMORY_INPUT(nes->memory) = &nes->input;
    CNES_MEMORY_PPU(nes->memory) = &nes->ppu;
    CNES_MEMORY_APU(nes->memory) = &nes->apu;
    cnes_cpu_init(&nes->cpu, &nes->memory);
    
    CLEAR(&nes->layoutp);
    CLEAR(&nes->memory_ppu);
    nes->memory_ppu.layout = &nes->layoutp;
    nes->memory_ppu.readbyte = cnes_layoutp_readbyte;
    nes->memory_ppu.readword = cnes_layoutp_readword;
    nes->memory_ppu.writebyte = cnes_layoutp_writebyte;
    nes->memory_ppu.writeword = cnes_layoutp_writeword;
    CNES_MEMORYP_CPU(nes->memory_ppu) = &nes->cpu;
    CNES_MEMORYP_PPU(nes->memory_ppu) = &nes->ppu;
    CNES_MEMORYP_MMC(nes->memory_ppu) = &nes->mmc;
    CLEAR(&nes->ppu);
    nes->ppu.mirroring = nes->rom.mirroring;
    nes->ppu.mapper = nes->rom.mapper;
    nes->ppu.memory = &nes->memory_ppu;
    nes->ppu.temp = 0xdddd;
    nes->ppu.address = 0xdddd;
    
    cnes_mmc_init(&nes->mmc, 
        &nes->rom, &nes->layout, &nes->layoutp, 
        &nes->cpu, &nes->ppu);
    
    CLEAR(&nes->video);
    const char* caption = basename(nes->conf.ppu.caption, nes->conf.emulator.file);
    result = cnes_video_open(&nes->video, caption, nes->conf.ppu.scale, 60.0f, 
        nes->conf.video.show_audio);
    if (FAILED(result)) {
        printf("Open video Error #1\n");
        cnes_rom_close(&nes->rom);
        cnes_cartridge_close(&nes->cartridge);
        return result;
    }
    result = cnes_video_start(&nes->video);
    if (FAILED(result)) {
        printf("Open video Error #2\n");
        cnes_rom_close(&nes->rom);
        cnes_cartridge_close(&nes->cartridge);
        return result;
    }
    
    CLEAR(&nes->apu);
    result = cnes_pAPU_apu_init(&nes->apu, &nes->memory, 
        nes->conf.apu.freq, nes->conf.apu.buffers, nes->conf.apu.stereo);
    if (FAILED(result)) {
        printf("Warning: audio device not open\n");
    }
    nes->apu.intensity = nes->conf.apu.intensity;
    nes->apu.volume = nes->conf.apu.volume;
    nes->apu.synth = nes->conf.apu.synth;
    nes->apu.dmc_vol = .0f;
    cnes_pAPU_apu_watch(&nes->apu, &nes->video.vdb);
    return SUCCESS;
}

static void timer_callback(cnes_timer_s* timer, cnes_nes_s* nes)
{
    BOOL finish_frame = FALSE;
    while(!finish_frame) {
        cnes_connector_read(&nes->connector, &nes->input);
        
        if (nes->input.QUIT) {
            nes->video.closed = TRUE;
            break;
        }
        if (nes->input.SAVE) {
            nes->input.SAVE = CNES_INPUT_KEYUP;
            cnes_save_state(nes);
        }
        if (nes->input.LOAD) {
            nes->input.LOAD = CNES_INPUT_KEYUP;
            cnes_load_state(nes);
        }
        
        cnes_cpu_cycle(&nes->cpu, 341/3, FALSE);
        finish_frame = cnes_ppu_line(&nes->ppu);
        memcpy(nes->video.frame, nes->ppu.image, 256*240);
        cnes_pAPU_apu_cycle(&nes->apu, 341/3, 60.0f);
    }
    nes->video.frame_ready = TRUE;
    if (nes->video.closed) {
        cnes_timer_signal(timer);
    }
}

void cnes_nes_run(cnes_nes_s* nes)
{
    cnes_timer_s timer;
    cnes_timer_init(&timer, 
        (cnes_timer_callback) timer_callback, nes, 
        1e6 / 60);
    cnes_timer_run(&timer);
}

void cnes_nes_close(cnes_nes_s* nes)
{
    cnes_connector_close(&nes->connector);
    cnes_pAPU_apu_close(&nes->apu);
    cnes_video_close(&nes->video);
    cnes_rom_close(&nes->rom);
    cnes_cartridge_close(&nes->cartridge);
}

void cnes_conf_init(cnes_conf_s* conf)
{
    CLEAR(conf);
    conf->ppu.scale = 2;
    conf->video.show_audio = FALSE;
    conf->apu.freq = 44100;
    conf->apu.buffers = 4;
    conf->apu.stereo = FALSE;
    conf->apu.intensity = 0.2;
    conf->apu.synth = S_LA;
    conf->apu.volume = 1.0;
}

static void ini_location(char* buffer)
{
    const char* home = getenv("HOME");
    assert(NOTNULL(home));
    strncat(buffer, home, FILENAME);
    strncat(buffer, "/.config/cnes.ini", FILENAME);
}

error_t cnes_read_ini(cnes_conf_s* conf)
{
    char filename[FILENAME];
    ini_location(filename);
    ini_file_s ini;
    if (FAILED(ini_open(&ini, filename))) {
        return OPEN_FILE_ERROR;
    }
    INI_PREPARE;
    INI_NUMERIC(&ini, &conf->ppu.scale);
    INI_BOOLEAN(&ini, &conf->video.show_audio);
    INI_NUMERIC(&ini, &conf->apu.freq);
    INI_NUMERIC(&ini, &conf->apu.buffers);
    INI_BOOLEAN(&ini, &conf->apu.stereo);
    INI_NUMERIC(&ini, &conf->apu.intensity);
    INI_NUMERIC(&ini, &conf->apu.synth);
    INI_NUMERIC(&ini, &conf->apu.volume);
    return SUCCESS;
}

void cnes_touch_ini(void)
{
    ini_file_s ini;
    if (FAILED(ini_open(&ini, NULL))) {
        return;
    }
    cnes_conf_s conf;
    (void) conf;
    ini_pair_s* pair;
    INI_PREPARE;
    INI_APPEND(&ini, conf.ppu.scale, "2.0");
    pair = INI_APPEND(&ini, conf.video.show_audio, "false");
    ini_pair_cmt(pair, " display apu status on top-left corner");
    INI_APPEND(&ini, conf.apu.freq, "44100");
    INI_APPEND(&ini, conf.apu.buffers, "4");
    INI_APPEND(&ini, conf.apu.stereo, "false");
    INI_APPEND(&ini, conf.apu.intensity, "2.0");
    pair = INI_APPEND(&ini, conf.apu.synth, "1");
    ini_pair_cmt(pair, " 0: S_FORMULAS, 1: S_LA, 2: S_LUT");
    pair = INI_APPEND(&ini, conf.apu.volume, "1.0");
    ini_pair_cmt(pair, " master volume [0, 1.0]");
    char filename[FILENAME];
    ini_location(filename);
    FILE* fout = fopen(filename, "w");
    if (NUL(fout)) {
        ini_close(&ini);
        return;
    }
    ini_dump(fout, &ini);
    ini_close(&ini);
    fclose(fout);
}

void cnes_edit_ini(BOOL x)
{
    char filename[FILENAME];
    ini_location(filename);
    char cmdline[BUFFER_LARGE];
    strncat(cmdline, x ? "xdg-open " : "vi ", BUFFER_LARGE);
    strncat(cmdline, filename, BUFFER_LARGE);
    system(cmdline);
}

error_t cnes_save_state(const cnes_nes_s* nes)
{
    return cnes_dump(nes, D_BINARY, DUMP_ALL);
}

error_t cnes_load_state(cnes_nes_s* nes)
{
    return cnes_load(nes, D_BINARY);
}