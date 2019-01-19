// pAPU.h
// Author: hyan23
// Date: 2017.08.21
// nesdev.com/apu_ref.txt

#ifndef __PAPU_H__
#define __PAPU_H__

#include "header.h"
#include "vdb.h"

// TODOs: 
// 1. The CPU's IRQ line is level-sensitie, so the APU's interrupt flags 
// must be cleared once a CPU IRQ is acknowledged, otherwise the CPU will 
// immediately be interrupt again once its inhibit flag is cleared.
// 2. At any time if the interrupt flag is set and the IRQ disable is clear, 
// The CPU's IRQ line is asserted.
// 3. Triangle channel: At the lowest two period(0x400b == 0 and 0x400a = 
// 0 or 1), the resulting frenquency is so high that the DAC effictively
// outpus a value half way between 7 and 8.
// 4. When the DMA reader accesses a byte of memory, the CPU is suspended
// for 4 clock cycles.
typedef void (*cnes_pAPU_divider_callback)(void* userdata);

typedef struct CNES_PAPU_DIVIDER
{
    uint    period;
    uint    counter;
    cnes_pAPU_divider_callback callback;
    void*   userdata;
}
cnes_pAPU_divider_s;

// callback: can be NULL
extern void cnes_pAPU_div_init(cnes_pAPU_divider_s* divider, 
    cnes_pAPU_divider_callback callback, void* userdata);
extern void cnes_pAPU_div_configure(cnes_pAPU_divider_s* divider, uint period);
extern void cnes_pAPU_div_reset(cnes_pAPU_divider_s* divider);
extern void cnes_pAPU_div_clock(cnes_pAPU_divider_s* divider);
extern uint cnes_pAPU_div_value(const cnes_pAPU_divider_s* divider);

// timer
// All channels use a timer which is a divider driven by the 1.79Mhz clock.
// The noise channel and DMC use lookup tables to set the timer's period.
// For the square and triangle channels, the third and fourth registers form 
// an 11-bit value and the divider's period is set to this value *plus 1*.
typedef cnes_pAPU_divider_callback cnes_pPAU_timer_callback;
typedef cnes_pAPU_divider_s cnes_pAPU_timer_s;

#define cnes_pAPU_timer_init cnes_pAPU_div_init
#define cnes_pAPU_timer_reset cnes_pAPU_div_reset
#define cnes_pAPU_timer_configure cnes_pAPU_div_configure
#define cnes_pAPU_timer_clock cnes_pAPU_div_clock

// sequencer
typedef void (*cnes_pAPU_sequencer_callback)(void* userdata, byte value);

typedef struct CNES_PAPU_SEQUENCER
{
    byte*   sequence;
    uint    seq_length;
    uint    seq_pos;
    byte    value;
    cnes_pAPU_sequencer_callback callback;
    void*   userdata;
}
cnes_pAPU_sequencer_s;

// callback: can be NULL
extern void cnes_pAPU_seq_init(cnes_pAPU_sequencer_s* sequencer, 
    cnes_pAPU_sequencer_callback callback, void* userdata);
extern void cnes_pAPU_seq_configure(cnes_pAPU_sequencer_s* sequencer, 
    byte* sequence, uint seq_length);
extern void cnes_pAPU_seq_reset(cnes_pAPU_sequencer_s* sequencer);
extern void cnes_pAPU_seq_clock(cnes_pAPU_sequencer_s* sequencer);
extern byte cnes_pAPU_seq_value(const cnes_pAPU_sequencer_s* sequencer);

// length counter
typedef struct CNES_PAPU_LENGTH_COUNTER
{
    BOOL    enabled;
    BOOL    halted;
    uint    counter;
}
cnes_pAPU_LC_s;

extern void cnes_pAPU_LC_init(cnes_pAPU_LC_s* lc);
extern void cnes_pAPU_LC_enable(cnes_pAPU_LC_s* lc, BOOL enabled);
extern void cnes_pAPU_LC_halt(cnes_pAPU_LC_s* lc, BOOL halted);
extern void cnes_pAPU_LC_load(cnes_pAPU_LC_s* lc, uint index);
extern void cnes_pAPU_LC_clock(cnes_pAPU_LC_s* lc);
extern uint cnes_aAPU_LC_value(const cnes_pAPU_LC_s* lc);

// envelope generator
typedef struct CNES_PAPU_ENVELOPE_GENERATOR
{
    cnes_pAPU_divider_s divider;
    uint    div_period;
    byte    counter;
    byte    constant_vol;
    BOOL    loop;
    BOOL    disabled;
    BOOL    latch;  /* fourth channel register write */
    byte    volume;
}
cnes_pAPU_EG_s;

extern void cnes_pAPU_EG_init(cnes_pAPU_EG_s* eg);
extern void cnes_pAPU_EG_configure(cnes_pAPU_EG_s* eg, 
    BOOL loop, BOOL disabled, byte n);
extern void cnes_pAPU_EG_clock(cnes_pAPU_EG_s* eg);
extern byte cnes_pAPU_EG_volume(cnes_pAPU_EG_s* eg);

// sweep unit
typedef struct CNES_PAPU_SWEEP_UNIT
{
    cnes_pAPU_divider_s divider;
    byte    div_period; /* 3 bits */
    BOOL    enabled;
    BOOL    negative;
    byte    shift;  /* 3 bits */
    uint    shifted_value;
    BOOL    latch;      /* sweep unit register write */
    BOOL    silence;
    uint    channel_number;
    uint*   channel_period;
    cnes_pAPU_timer_s*  channel_timer;
}
cnes_pAPU_SU_s;

extern void cnes_pAPU_SU_init(cnes_pAPU_SU_s* su, 
    uint channel_number, uint* channel_period, cnes_pAPU_timer_s* channel_timer);
extern void cnes_pAPU_SU_configure(cnes_pAPU_SU_s* su, 
    BOOL enabled, byte p, BOOL negative, byte shift);
extern void cnes_pAPU_SU_clock(cnes_pAPU_SU_s* su);

// square channel
typedef struct CNES_PAPU_SQUARE_CHANNEL
{
    cnes_pAPU_EG_s  eg;
    cnes_pAPU_SU_s  su;
    cnes_pAPU_timer_s timer;
    cnes_pAPU_sequencer_s sequencer;
    cnes_pAPU_LC_s  lc;
    uint    channel_number;
    uint    period;
    byte    r0;
    byte    r1;
    byte    r2;
    byte    r3;
    byte    dac;
}
cnes_pAPU_SC_s;

extern void cnes_pAPU_SC_init(cnes_pAPU_SC_s* sc, uint channel_number);
extern void cnes_pAPU_SC_write(cnes_pAPU_SC_s* sc, uint reg, byte value);
extern byte cnes_pAPU_SC_DAC(const cnes_pAPU_SC_s* sc);

// linear counter
typedef struct CNES_PAPU_LINEAR_COUNTER
{
    BOOL    control;
    BOOL    halted; /* when register $400b is written, set flag */
    uint    counter;
    uint    reload_value;
}
cnes_pAPU_linear_s;

extern void cnes_pAPU_linear_init(cnes_pAPU_linear_s* linear);
extern void cnes_pAPU_linear_configure(cnes_pAPU_linear_s* linear, 
    BOOL control, uint reload_value);
extern void cnes_pAPU_linear_halt(cnes_pAPU_linear_s* linear, BOOL halted);
extern void cnes_pAPU_linear_clock(cnes_pAPU_linear_s* linear);
extern uint cnes_pAPU_linear_value(const cnes_pAPU_linear_s* linear);

// triangle channel
typedef struct CNES_PAPU_TRIANGLE_CHANNEL
{
    cnes_pAPU_timer_s   timer;
    cnes_pAPU_sequencer_s sequencer;
    cnes_pAPU_LC_s  lc;
    cnes_pAPU_linear_s linear;
    uint    period;
    byte    r0;
    byte    r2;
    byte    r3;
    byte    dac;    /* 4 bits */
}
cnes_pAPU_TC_s;

extern void cnes_pAPU_TC_init(cnes_pAPU_TC_s* tc);
extern void cnes_pAPU_TC_write(cnes_pAPU_TC_s* tc, uint reg, byte value);
extern byte cnes_pAPU_TC_DAC(const cnes_pAPU_TC_s* tc);

// noise channel
typedef struct CNES_PAPU_NOISE_CHANNEL
{
    cnes_pAPU_LC_s  lc;
    cnes_pAPU_EG_s  eg;
    cnes_pAPU_timer_s timer;
    word    right_shift;    /* 15 bits */
    BOOL    short_mode;
    byte    r0;
    byte    r2;
    byte    r3;
    byte    dac;    /* 4 bits */
}
cnes_pAPU_NC_s;

extern void cnes_pAPU_NC_init(cnes_pAPU_NC_s* nc);
extern void cnes_pAPU_NC_write(cnes_pAPU_NC_s* nc, uint reg, byte value);
extern byte cnes_pAPU_NC_DAC(cnes_pAPU_NC_s* nc);

// DMA Reader, Output Unit, DMC Channel *Not tested*
// TODO: test
#include "../6502/memory.h"

typedef struct CNES_PAPU_DMA_READER
{
    word    address_counter;
    uint    bytes;
    cnes_memory_s*  memory;
}
cnes_pAPU_DR_s;

extern void cnes_pAPU_DR_init(cnes_pAPU_DR_s* dr, cnes_memory_s* memory);
extern void cnes_pAPU_DR_configure(cnes_pAPU_DR_s* dr, word addr, uint bytes);
extern byte cnes_pAPU_DR_read(cnes_pAPU_DR_s* dr);

// Output Unit
typedef struct CNES_PAPU_OUTPUT_UNIT
{
    byte    right_shift;    /* 8 bits */
    uint    counter;
    BOOL    silence;
}
cnes_pAPU_OU_s;

extern void cnes_pAPU_OU_init(cnes_pAPU_OU_s* ou);

typedef struct CNES_PAPU_DMC
{
    cnes_memory_s*  memory;
    cnes_pAPU_DR_s  dr;
    BOOL    irqEnabled;
    BOOL    iFlag;
    BOOL    loop;
    BOOL    sample_buf_empty;
    byte    sample_buffer;
    cnes_pAPU_timer_s   timer;
    cnes_pAPU_OU_s  ou;
    byte    counter;    /* 7 bits */
    byte    r0;
    byte    r1;
    byte    r2;
    byte    r3;
    byte    dac;    /* 7 bits */
}
cnes_pAPU_DMC_s;

extern void cnes_pAPU_DMC_init(cnes_pAPU_DMC_s* dmc, cnes_memory_s* memory);
extern void cnes_pAPU_DMC_write(cnes_pAPU_DMC_s* dmc, uint reg, byte value);
extern void cnes_pAPU_DMC_clock(cnes_pAPU_DMC_s* dmc);
extern byte cnes_pAPU_DMC_DAC(cnes_pAPU_DMC_s* dmc);

// frame sequencer
typedef struct CNES_PAPU_FRAME_SEQUENCER
{
    cnes_pAPU_divider_s divider;
    cnes_pAPU_sequencer_s   sequencer;
    uint    mode;
    BOOL    irqEnabled;
    BOOL    iFlag;
    cnes_pAPU_SC_s  sc1;
    cnes_pAPU_SC_s  sc2;
    cnes_pAPU_TC_s  tc;
    cnes_pAPU_NC_s  nc;
    cnes_pAPU_DMC_s dmc;
    cnes_memory_s*  memory;
}
cnes_pAPU_frame_seq_s;

extern void cnes_pAPU_FS_init(cnes_pAPU_frame_seq_s* frame_seq, 
    cnes_memory_s* memory);
extern void cnes_pAPU_FS_configure(cnes_pAPU_frame_seq_s* frame_seq, 
    uint mode, BOOL irqEnabled);
extern void cnes_pAPU_FS_clock(cnes_pAPU_frame_seq_s* frame_seq);

// APU
#include "audio.h"

typedef enum CNES_PAPU_APU_SYNTH {
    S_FORMULAS, S_LA,   S_LUT
}
cnes_pAPU_apu_synth_t;

typedef enum CNES_PAPU_APU_CHANNEL {
    C_MONO, C_LEFT, C_RIGHT
}
cnes_pAPU_apu_channel_t;

typedef struct CNES_PAPU_APU
{
    cnes_pAPU_frame_seq_s   frame_seq;
    cnes_audio_s    audio;
    uint    apu_cycles;
    uint    apuCyclesPerSecond;
    uint    apuCyclesPerSample;
    float   volume;
    float   sc1_vol;
    float   sc2_vol;
    float   tc_vol;
    float   nc_vol;
    float   dmc_vol;
    byte    sc1_dac;    /* units volume applied */
    byte    sc2_dac;
    byte    tc_dac;
    byte    nc_dac;
    byte    dmc_dac;
    cnes_pAPU_apu_synth_t   synth;
    sint    freq;
    sint    buffers;
    BOOL    stereo;
    float   intensity;
    float   dac;    /* mixed */
}
cnes_pAPU_apu_s;

extern error_t cnes_pAPU_apu_init(cnes_pAPU_apu_s* apu, 
    cnes_memory_s* memory, uint freq, uint buffers, BOOL stereo);
extern void cnes_pAPU_apu_close(cnes_pAPU_apu_s* apu);
extern void cnes_pAPU_apu_dumpb(FILE* fout, const cnes_pAPU_apu_s* apu);
extern void cnes_pAPU_apu_loadb(cnes_pAPU_apu_s* apu, FILE* fin);
extern void cnes_pAPU_apu_cycle(cnes_pAPU_apu_s* apu, uint apu_cycle, float fps);
extern byte cnes_pAPU_apu_read(cnes_pAPU_apu_s* apu, word addr);
extern void cnes_pAPU_apu_write(cnes_pAPU_apu_s* apu, word addr, byte value);
extern void cnes_pAPU_apu_watch(cnes_pAPU_apu_s* apu, cnes_vdb_s* vdb);
extern float cnes_pAPU_apu_DAC(cnes_pAPU_apu_s* apu, 
    cnes_pAPU_apu_channel_t channel);

#endif /* __PAPU_H__ */