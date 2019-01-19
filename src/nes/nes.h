// nes.h
// Author: hyan23
// Date: 2018.05.28

#ifndef __NES_H__
#define __NES_H__

#include "header.h"
#include "common.h"
#include "../rom/cartridge.h"
#include "../rom/rom.h"
#include "../input/connector.h"
#include "../input/input.h"
#include "../6502/memory.h"
#include "../6502/cpu.h"
#include "../2C02/layoutp.h"
#include "../2C02/ppu.h"
#include "../mmc/mmc.h"
#include "../2A03/layout.h"
#include "../2A03/pAPU.h"
#include "../2C02/video.h"

typedef struct CNES_CONF
{
    /* emulator */
    struct {
        char    file[FILENAME];
    } emulator;
    /* ppu */
    struct {
        float   scale;
        char    caption[BUFFER_MIDDLE];
    } ppu;
    /* video */
    struct {
        BOOL    show_audio;
    } video;
    /* apu */
    struct {
        uint    freq;
        uint    buffers;
        BOOL    stereo;
        float   intensity;
        cnes_pAPU_apu_synth_t   synth;
        float   volume;
    } apu;
}
cnes_conf_s;

extern void cnes_conf_init(cnes_conf_s* conf);
extern error_t cnes_read_ini(cnes_conf_s* conf);
extern void cnes_touch_ini(void);
extern void cnes_edit_ini(BOOL x);

typedef struct CNES_NES
{
    cnes_conf_s conf;
    cnes_cartridge_s    cartridge;
    cnes_rom_s  rom;
    cnes_connector_s    connector;
    cnes_input_s        input;      // *
    cnes_layout_s   layout;     // *
    cnes_memory_s   memory;
    cnes_cpu_s      cpu;        // *
    cnes_layoutp_s  layoutp;        // *
    cnes_memory_s   memory_ppu;
    cnes_ppu_s      ppu;        //  *
    cnes_mmc_s  mmc;
    cnes_pAPU_apu_s apu;    // *
    cnes_video_s    video;
}
cnes_nes_s;

extern error_t cnes_nes_init(cnes_nes_s* nes, const cnes_conf_s* conf);
extern void cnes_nes_run(cnes_nes_s* nes);
extern void cnes_nes_close(cnes_nes_s* nes);
extern error_t cnes_save_state(const cnes_nes_s* nes);
extern error_t cnes_load_state(cnes_nes_s* nes);

#endif /* __NES_H__ */