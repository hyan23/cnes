// pAPU.c
// Author: hyan23
// Date: 2017.08.20

#include "header.h"
#include "common.h"
#include "pAPU.h"

// divider
void 
cnes_pAPU_div_init(cnes_pAPU_divider_s* divider, 
cnes_pAPU_divider_callback callback, void* userdata)
{
    divider->callback = callback;
    divider->userdata = userdata;
    divider->period = 1; // Not sure
    divider->counter = divider->period;
}

void cnes_pAPU_div_configure(cnes_pAPU_divider_s* divider, uint period)
{
    divider->period = period;
}

void cnes_pAPU_div_reset(cnes_pAPU_divider_s* divider)
{
    divider->counter = divider->period;
}

void cnes_pAPU_div_clock(cnes_pAPU_divider_s* divider)
{
    if (divider->counter > 0) {
        divider->counter --;
    }
    if (divider->counter == 0) {
        if (NOTNULL(divider->callback)) {
            divider->callback(divider->userdata);
        }
        divider->counter = divider->period;
    }
}

uint cnes_pAPU_div_value(const cnes_pAPU_divider_s* divider)
{
    return divider->counter;
}

// sequencer
void 
cnes_pAPU_seq_init(cnes_pAPU_sequencer_s* sequencer, 
cnes_pAPU_sequencer_callback callback, void* userdata)
{
    sequencer->callback = callback;
    sequencer->userdata = userdata;
    sequencer->sequence = NULL;
    sequencer->seq_length = 0;
    sequencer->seq_pos = 0;
}

void cnes_pAPU_seq_reset(cnes_pAPU_sequencer_s* sequencer)
{
    sequencer->seq_pos = 0;
}

void 
cnes_pAPU_seq_configure(cnes_pAPU_sequencer_s* sequencer, 
byte* sequence, uint seq_length)
{
    sequencer->sequence = sequence;
    sequencer->seq_length = seq_length;
    sequencer->value = 0;
}

void cnes_pAPU_seq_clock(cnes_pAPU_sequencer_s* sequencer)
{
    if (NOTNULL(sequencer->sequence)) {
        sequencer->value = sequencer->sequence[sequencer->seq_pos];
        if (NOTNULL(sequencer->callback)) {
            sequencer->callback(sequencer->userdata, sequencer->value);
        }
    }
    if (sequencer->seq_pos < sequencer->seq_length) {
        sequencer->seq_pos ++;
    }
    if (sequencer->seq_pos /*==*/ >= sequencer->seq_length) {
        sequencer->seq_pos = 0;
    }
}

byte cnes_pAPU_seq_value(const cnes_pAPU_sequencer_s* sequencer)
{
    return sequencer->value;
}

// length counter
void cnes_pAPU_LC_init(cnes_pAPU_LC_s* lc)
{
    lc->enabled = FALSE; // Not sure
    lc->halted = FALSE;
    lc->counter = 0;
}

void cnes_pAPU_LC_enable(cnes_pAPU_LC_s* lc, BOOL enabled)
{
    lc->enabled = enabled;
    if (!lc->enabled) {
        lc->counter = 0;
    }
}

void cnes_pAPU_LC_halt(cnes_pAPU_LC_s* lc, BOOL halted)
{
    lc->halted = halted;
}

static byte LC_reload_values[] = {
    0x0a, 0xfe, 0x14, 0x02, 0x28, 0x04, 0x50, 0x06, 
    0xa0, 0x08, 0x3c, 0x0a, 0x0e, 0x0c, 0x1a, 0x0e, 
    0x0c, 0x10, 0x18, 0x12, 0x30, 0x14, 0x60, 0x16, 
    0xc0, 0x18, 0x48, 0x1a, 0x10, 0x1c, 0x20, 0x1e
};

void cnes_pAPU_LC_load(cnes_pAPU_LC_s* lc, uint index)
{
    assert(WIDTH(index, 5));
    if (lc->enabled) {
        lc->counter = LC_reload_values[index];
    }
}

void cnes_pAPU_LC_clock(cnes_pAPU_LC_s* lc)
{
    if (!lc->halted && lc->counter > 0) {
        lc->counter --;
    }
}

uint cnes_aAPU_LC_value(const cnes_pAPU_LC_s* lc)
{
    return lc->counter;
}

// envolope generator
static void EG_div_callback(cnes_pAPU_EG_s* eg)
{
    if (eg->loop && eg->counter == 0) {
        eg->counter = 15;
    } else {
        if (eg->counter > 0) {
            eg->counter --;
        }
    }
}

void cnes_pAPU_EG_init(cnes_pAPU_EG_s* eg)
{
    cnes_pAPU_div_init(&eg->divider, 
        (cnes_pAPU_divider_callback) EG_div_callback, eg);
    eg->counter = 0;    // Not sure
    eg->div_period = 1; // Not sure
    eg->constant_vol = 0;   // Not sure
    eg->loop = FALSE;   // Not sure
    eg->disabled = TRUE;    // Not sure
    eg->latch = FALSE;
    eg->volume = 0;
}

void 
cnes_pAPU_EG_configure(cnes_pAPU_EG_s* eg, 
BOOL loop, BOOL disabled, byte n)
{
    eg->loop = loop;
    eg->disabled = disabled;
    assert(WIDTH(n, 4));
    eg->div_period = n + 1;
    eg->constant_vol = n;
    cnes_pAPU_div_configure(&eg->divider, eg->div_period);
}

void cnes_pAPU_EG_clock(cnes_pAPU_EG_s* eg)
{
    if (eg->latch) {
        eg->counter = 15;
        cnes_pAPU_div_reset(&eg->divider);
        eg->latch = FALSE;
    } else {
        cnes_pAPU_div_clock(&eg->divider);
    }
}

byte cnes_pAPU_EG_volume(cnes_pAPU_EG_s* eg)
{
    if (eg->disabled) {
        eg->volume = eg->constant_vol;
    } else {
        eg->volume = eg->counter;
    }
    assert(WIDTH(eg->volume, 4));
    return eg->volume;
}

// sweep unit
void SU_div_callback(cnes_pAPU_SU_s* su)
{
    if (su->enabled && su->shift > 0 && !su->silence) {
        *su->channel_period = su->shifted_value;
        cnes_pAPU_timer_configure(su->channel_timer, *su->channel_period);
    }
}

void
cnes_pAPU_SU_init(cnes_pAPU_SU_s* su, 
uint channel_number, uint* channel_period, cnes_pAPU_timer_s* channel_timer)
{
    assert(channel_number == 1 || channel_number == 2);
    su->channel_number = channel_number;
    assert(NOTNULL(channel_period));
    su->channel_period = channel_period;
    assert(NOTNULL(channel_timer));
    su->channel_timer = channel_timer;
    cnes_pAPU_div_init(&su->divider, 
        (cnes_pAPU_divider_callback) SU_div_callback, su);
    su->enabled = FALSE;    // Not sure
    su->div_period = 1;     // Not sure
    su->negative = FALSE;
    su->shift = 0;      // Not sure
    su->shifted_value = 0;
    su->latch = FALSE;
    su->silence = FALSE;
}

void
cnes_pAPU_SU_configure(cnes_pAPU_SU_s* su, 
BOOL enabled, byte p, BOOL negative, byte shift)
{
    su->latch = TRUE;
    su->enabled = enabled;
    su->negative = negative;
    assert(WIDTH(shift, 3));
    su->shift = shift;
    assert(WIDTH(p, 3));
    su->div_period = p + 1;
    cnes_pAPU_div_configure(&su->divider, su->div_period);
}

static uint SU_shifter_shift(const cnes_pAPU_SU_s* su)
{
    uint shifted_value = *su->channel_period >> su->shift;
    if (su->negative) {
        shifted_value = *su->channel_period - shifted_value + 
            (su->channel_number == 2 ? 1 : 0);
    } else {
        shifted_value += *su->channel_period;
    }
    return shifted_value;
}

void cnes_pAPU_SU_clock(cnes_pAPU_SU_s* su)
{
    su->shifted_value = SU_shifter_shift(su);
    if (*su->channel_period < 8 || su->shifted_value > 0x7ff) {
        su->silence = TRUE;
    } else {
        su->silence = FALSE;
    }
    cnes_pAPU_div_clock(&su->divider);
    if (su->latch) {
        cnes_pAPU_div_reset(&su->divider);
        su->latch = FALSE;
    }
}

// square channel
static void SC_timer_callback(cnes_pAPU_SC_s* sc)
{
    cnes_pAPU_seq_clock(&sc->sequencer);
}

static void SC_seq_callback(cnes_pAPU_SC_s* sc)
{
    if (sc->lc.counter == 0 || sc->su.silence) {
        sc->dac = 0;
    } else {
        sc->dac = cnes_pAPU_seq_value(&sc->sequencer) * 
            cnes_pAPU_EG_volume(&sc->eg);
    }
}

void cnes_pAPU_SC_init(cnes_pAPU_SC_s* sc, uint channel_number)
{
    assert(channel_number == 1 || channel_number == 2);
    sc->channel_number = channel_number;
    cnes_pAPU_EG_init(&sc->eg);
    cnes_pAPU_SU_init(&sc->su, sc->channel_number, &sc->period, &sc->timer);
    cnes_pAPU_timer_init(&sc->timer, 
        (cnes_pPAU_timer_callback) SC_timer_callback, sc);
    cnes_pAPU_seq_init(&sc->sequencer, 
        (cnes_pAPU_sequencer_callback) SC_seq_callback, sc);
    cnes_pAPU_LC_init(&sc->lc);
    sc->period = 1; // Not sure
    sc->r0 = 0;
    sc->r1 = 0;
    sc->r2 = 0;
    sc->r3 = 0;
    sc->dac = 0;
}

static byte SC_waveform_seqs[4][8] = {
    { 0, 1, 0, 0, 0, 0, 0, 0 },     /* 12.5% */
    { 0, 1, 1, 0, 0, 0, 0, 0 },     /* 25% */
    { 0, 1, 1, 1, 1, 0, 0, 0 },     /* 50% */
    { 1, 0, 0, 1, 1, 1, 1, 1 }      /* 25% nagated */
};

#define SC_R0_SEQ_DUTY(r0)  ((r0) >> 6)
#define SC_R0_ENV_LOOP(r0)  test_bit((r0), 5)
#define SC_R0_LC_HALTED     SC_R0_ENV_LOOP
#define SC_R0_ENV_DISABLED(r0)  test_bit((r0), 4)
#define SC_R0_N(r0)         ((r0) & 0x0f)

#define SC_R1_SU_ENABLED(r1)    test_bit((r1), 7)
#define SC_R1_SU_PERIOD(r1) (((r1) >> 4) & 0x07)
#define SC_R1_SU_NAGATIVE(r1)   test_bit((r1), 3)
#define SC_R1_SU_SHIFT(r1)  ((r1) & 0x07)

#define SC_R2_CHAN_PERIOD_LOW(r2)   r2

#define SC_R3_LC_INDEX(r3)  ((r3) >> 3)
#define SC_R3_CHAN_PERIOD_HIGH(r3)  ((r3) & 0x07)

void cnes_pAPU_SC_write(cnes_pAPU_SC_s* sc, uint reg, byte value)
{
    switch (reg) {
        case 0: {
            sc->r0 = value;
            cnes_pAPU_seq_configure(&sc->sequencer, 
                SC_waveform_seqs[SC_R0_SEQ_DUTY(value)], 8);
            cnes_pAPU_LC_halt(&sc->lc, SC_R0_LC_HALTED(value));
            cnes_pAPU_EG_configure(&sc->eg, 
                SC_R0_ENV_LOOP(value), 
                SC_R0_ENV_DISABLED(value), 
                SC_R0_N(value));
            break;
        }
        case 1: {
            sc->r1 = value;
            cnes_pAPU_SU_configure(&sc->su, 
                SC_R1_SU_ENABLED(value), 
                SC_R1_SU_PERIOD(value), 
                SC_R1_SU_NAGATIVE(value), 
                SC_R1_SU_SHIFT(value));
            break;
        }
        case 2: {
            sc->r2 = value;
            sc->period &= ~0xff;
            sc->period |= SC_R2_CHAN_PERIOD_LOW(value);
            cnes_pAPU_timer_configure(&sc->timer, sc->period + 1);
            break;
        }
        case 3: {
            sc->r3 = value;
            sc->period &= 0xff;
            sc->period |= SC_R3_CHAN_PERIOD_HIGH(value) << 8;
            cnes_pAPU_timer_configure(&sc->timer, sc->period + 1);
            cnes_pAPU_LC_load(&sc->lc, SC_R3_LC_INDEX(value));
            sc->eg.latch = TRUE;
            break;
        }
        default:
            // Illegal access
            break;
    }
}

byte cnes_pAPU_SC_DAC(const cnes_pAPU_SC_s* sc)
{
    assert(WIDTH(sc->dac, 4));
    return sc->dac;
}

// linear counter
void cnes_pAPU_linear_init(cnes_pAPU_linear_s* linear)
{
    linear->control = FALSE;    // Not sure
    linear->halted = FALSE; // Not sure
    linear->counter = 0;
    linear->reload_value = 0;
}

void
cnes_pAPU_linear_configure(cnes_pAPU_linear_s* linear, 
BOOL control, uint reload_value)
{
    linear->control = control;
    assert(WIDTH(reload_value, 7));
    linear->reload_value = reload_value;
}

void cnes_pAPU_linear_halt(cnes_pAPU_linear_s* linear, BOOL halted)
{
    linear->halted = halted;
}

void cnes_pAPU_linear_clock(cnes_pAPU_linear_s* linear)
{
    if (linear->halted) {
        linear->counter = linear->reload_value;
    } else {
        if (linear->counter > 0) {
            linear->counter --;
        }
    }
    if (!linear->control) {
        linear->halted = FALSE;
    }
}

uint cnes_pAPU_linear_value(const cnes_pAPU_linear_s* linear)
{
    return linear->counter;
}

// triangle channel
static byte TC_waveform_seqs[32] = {
    0x0f,   0x0e,   0x0d,   0x0c,   0x0b,   0x0a,   0x09,   0x08, 
    0x07,   0x06,   0x05,   0x04,   0x03,   0x02,   0x01,   0x0, 
    0x0,    0x01,   0x02,   0x03,   0x04,   0x05,   0x06,   0x07, 
    0x08,   0x09,   0x0a,   0x0b,   0x0c,   0x0d,   0x0e,   0x0f
};

static void TC_timer_callback(cnes_pAPU_TC_s* tc)
{
    if (tc->timer.period > 1 && tc->lc.counter != 0 && 
        tc->linear.counter != 0) {
        cnes_pAPU_seq_clock(&tc->sequencer);
    }
}

static void TC_seq_callback(cnes_pAPU_TC_s* tc)
{
    // if (tc->lc.counter == 0 || tc->linear.counter == 0) {
    //     tc->dac = 0;
    // } else {
        tc->dac = cnes_pAPU_seq_value(&tc->sequencer);
    // }
}

void cnes_pAPU_TC_init(cnes_pAPU_TC_s* tc)
{
    cnes_pAPU_timer_init(&tc->timer, 
        (cnes_pPAU_timer_callback) TC_timer_callback, tc);
    cnes_pAPU_seq_init(&tc->sequencer, 
        (cnes_pAPU_sequencer_callback) TC_seq_callback, tc);
    cnes_pAPU_seq_configure(&tc->sequencer, TC_waveform_seqs, 32);
    cnes_pAPU_LC_init(&tc->lc);
    cnes_pAPU_linear_init(&tc->linear);
    tc->period = 1; // Not sure
    tc->dac = 0;
    tc->r0 = 0;
    tc->r2 = 0;
    tc->r3 = 0;
}

#define TC_R0_LINEAR_CTRL(r0)   test_bit((r0), 7)
#define TC_R0_LC_HALTED         TC_R0_LINEAR_CTRL
#define TC_R0_LINEAR_RELOAD_VAL(r0) ((r0) & 0x7f)
#define TC_R2_PERIOD_LOW(r2)    r2
#define TC_R3_LC_INDEX(r3)      ((r3) >> 3)
#define TC_R3_PERIOD_HIGH(r3)   ((r3) & 0x07)

void cnes_pAPU_TC_write(cnes_pAPU_TC_s* tc, uint reg, byte value)
{
    switch (reg) {
        case 0: {
            tc->r0 = value;
            cnes_pAPU_LC_halt(&tc->lc, TC_R0_LC_HALTED(value));
            cnes_pAPU_linear_configure(&tc->linear, 
                TC_R0_LINEAR_CTRL(value), 
                TC_R0_LINEAR_RELOAD_VAL(value));
            break;
        }
        case 2: {
            tc->r2 = value;
            tc->period &= ~0xff;
            tc->period |= TC_R2_PERIOD_LOW(value);
            cnes_pAPU_timer_configure(&tc->timer, tc->period + 1);
            break;
        }
        case 3: {
            tc->r3 = value;
            cnes_pAPU_linear_halt(&tc->linear, TRUE);
            cnes_pAPU_LC_load(&tc->lc, TC_R3_LC_INDEX(value));
            tc->period &= 0xff;
            tc->period |= TC_R3_PERIOD_HIGH(value) << 8;
            cnes_pAPU_timer_configure(&tc->timer, tc->period + 1);
            break;
        }
        default:
            // Illegal access
            break;
    }
}

byte cnes_pAPU_TC_DAC(const cnes_pAPU_TC_s* tc)
{
    assert(WIDTH(tc->dac, 4));
    return tc->dac;
}

// noise channel
void NC_timer_callback(cnes_pAPU_NC_s* nc)
{
    word pre_shifted = nc->right_shift;
    word bit14 = test_bit(pre_shifted, 0) ^ 
        (nc->short_mode == 0 ? 
            (test_bit(pre_shifted, 1) >> 1) : 
            (test_bit(pre_shifted, 6) >> 6));
    nc->right_shift = nc->right_shift >> 1 | bit14 << 14;
}

void cnes_pAPU_NC_init(cnes_pAPU_NC_s* nc)
{
    cnes_pAPU_LC_init(&nc->lc);
    cnes_pAPU_EG_init(&nc->eg);
    cnes_pAPU_timer_init(&nc->timer, 
        (cnes_pPAU_timer_callback) NC_timer_callback, nc);
    nc->right_shift = 1;
    nc->dac = 0;
    nc->r0 = 0;
    nc->r2 = 0;
    nc->r3 = 0;
}

#define NC_R0_ENV_LOOP(r0)      test_bit((r0), 5)
#define NC_R0_LC_HALTED         NC_R0_ENV_LOOP
#define NC_R0_ENV_DISABLED(r0)  test_bit((r0), 4)
#define NC_R0_ENV_N(r0)         ((r0) & 0x0f)
#define NC_R2_SHORT_MODE(r2)    test_bit((r2), 7)
#define NC_R2_PERIOD_INDEX(r2)  ((r2) & 0x0f)
#define NC_R3_LC_INDEX(r3)      ((r3) >> 3)

static uint NC_timer_periods[0x10] = {
    0x004, 0x008, 0x010, 0x020, 
    0x040, 0x060, 0x080, 0x0a0, 
    0x0ca, 0x0fe, 0x17c, 0x1fc, 
    0x2fa, 0x3f8, 0x7f2, 0xfe4
};

void cnes_pAPU_NC_write(cnes_pAPU_NC_s* nc, uint reg, byte value)
{
    switch (reg) {
        case 0: {
            nc->r0 = value;
            cnes_pAPU_EG_configure(&nc->eg, 
                NC_R0_ENV_LOOP(value), 
                NC_R0_ENV_DISABLED(value), 
                NC_R0_ENV_N(value));
            cnes_pAPU_LC_halt(&nc->lc, NC_R0_LC_HALTED(value));
            break;
        }
        case 2: {
            nc->r2 = value;
            nc->short_mode = NC_R2_SHORT_MODE(value);
            cnes_pAPU_timer_configure(&nc->timer, 
                NC_timer_periods[NC_R2_PERIOD_INDEX(value)]);
            break;
        }
        case 3: {
            nc->r3 = value;
            cnes_pAPU_LC_load(&nc->lc, NC_R3_LC_INDEX(value));
            nc->eg.latch = TRUE;
            break;
        }
        default:
            // Illegal access
            break;
    }
}

byte cnes_pAPU_NC_DAC(cnes_pAPU_NC_s* nc)
{
    if (nc->lc.counter == 0 || test_bit(nc->right_shift, 0)) {
        nc->dac = 0;
    } else {
        nc->dac = cnes_pAPU_EG_volume(&nc->eg);
    }
    assert(WIDTH(nc->dac, 4));
    return nc->dac;
}

// DMA Reader
void cnes_pAPU_DR_init(cnes_pAPU_DR_s* dr, cnes_memory_s* memory)
{
    dr->memory = memory;
    dr->address_counter = 0;    // Not sure
    dr->bytes = 0;
}

void cnes_pAPU_DR_configure(cnes_pAPU_DR_s* dr, word addr, uint bytes)
{
    dr->address_counter = addr;
    dr->bytes = bytes;
}

byte cnes_pAPU_DR_read(cnes_pAPU_DR_s* dr)
{
    byte byte_read = dr->memory->readbyte(dr->memory, dr->address_counter);
    if (dr->address_counter < 0xffff) {
        dr->address_counter += 1;
    } else {
        dr->address_counter = 0x8000;
    }
    if (dr->bytes > 0) {
        dr->bytes --;
    }
    return byte_read;
}

// Output unit
void cnes_pAPU_OU_init(cnes_pAPU_OU_s* ou)
{
    ou->right_shift = 0;
    ou->counter = 0;
    ou->silence = FALSE;    // Not sure
}

// DMC
static void DMC_output_cycle(cnes_pAPU_DMC_s* dmc)
{
    dmc->ou.counter = 8;
    if (dmc->sample_buf_empty) {
        dmc->ou.silence = TRUE;
    } else {
        dmc->ou.silence = FALSE;
        dmc->ou.right_shift = dmc->sample_buffer;
        dmc->sample_buf_empty = TRUE;
    }
}

void DMC_timer_callback(cnes_pAPU_DMC_s* dmc)
{
    if (!dmc->ou.silence) {
        if (!test_bit(dmc->ou.right_shift, 0)) {
            if (dmc->counter > 1) {
                dmc->counter -= 2;
            }
        } else {
            if (dmc->counter < 126) {
                dmc->counter += 2;
            }
        }
        dmc->ou.right_shift >>= 1;
        dmc->ou.counter --;
    }
    if (dmc->ou.counter == 0) {
        DMC_output_cycle(dmc);
    }
}

static void DMC_restart(cnes_pAPU_DMC_s* dmc)
{
    cnes_pAPU_DR_configure(&dmc->dr, 
        0xc000 + dmc->r2 * 0x40, 
        dmc->r3 * 0x10 + 1);
}

void cnes_pAPU_DMC_init(cnes_pAPU_DMC_s* dmc, cnes_memory_s* memory)
{
    assert(NOTNULL(memory));
    dmc->memory = memory;
    cnes_pAPU_DR_init(&dmc->dr, dmc->memory);
    dmc->irqEnabled = FALSE;    // Not sure
    dmc->iFlag = FALSE;
    dmc->loop = FALSE;  // Not sure
    dmc->sample_buf_empty = TRUE;
    dmc->sample_buffer = 0;
    cnes_pAPU_timer_init(&dmc->timer, 
        (cnes_pPAU_timer_callback) DMC_timer_callback, dmc);
    cnes_pAPU_OU_init(&dmc->ou);
    dmc->counter = 0;
    dmc->r0 = 0;
    dmc->r1 = 0;
    dmc->r2 = 0;
    dmc->r3 = 0;
    dmc->dac = 0;
}

#define DMC_R0_IRQ_ENABLED(r0)  test_bit((r0), 7)
#define DMC_R0_LOOP(r0)         test_bit((r0), 6)
#define DMC_R0_FREQ_INDEX(r0)   ((r0) & 0x0f)
#define DMC_R1_DAC(r1)          ((r1) & 0x7f)
#define DMC_R2_SAMPLE_ADD(r2)   r2
#define DMC_R3_SAMPLE_LEN(r3)   r3

static uint DMC_timer_periods[0x10] = {
    0x1ac, 0x17c, 0x154, 0x140, 0x11e, 0x0fe, 0x0e2, 0x0d6, 
    0x0be, 0x0a0, 0x08e, 0x080, 0x06a, 0x054, 0x048, 0x036
};

void cnes_pAPU_DMC_write(cnes_pAPU_DMC_s* dmc, uint reg, byte value)
{
    switch (reg) {
        case 0: {
            dmc->r0 = value;
            dmc->irqEnabled = DMC_R0_IRQ_ENABLED(value);
            if (!dmc->irqEnabled) {
                dmc->iFlag = FALSE;
            }
            dmc->loop = DMC_R0_LOOP(value);
            cnes_pAPU_timer_configure(&dmc->timer, 
                DMC_timer_periods[DMC_R0_FREQ_INDEX(value)]);
            break;
        }
        case 1: {
            dmc->r1 = 0;
            dmc->counter = DMC_R1_DAC(value);
            dmc->dac = dmc->counter;
            break;
        }
        case 2: {
            dmc->r2 = DMC_R2_SAMPLE_ADD(value);
            break;
        }
        case 3: {
            dmc->r3 = DMC_R3_SAMPLE_LEN(value);
            break;
        }
        default:
            // Illegal access
            break;
    }
}

// Not sure
void cnes_pAPU_DMC_clock(cnes_pAPU_DMC_s* dmc)
{
    if (dmc->sample_buf_empty && dmc->dr.bytes != 0) {
        dmc->sample_buffer = cnes_pAPU_DR_read(&dmc->dr);
        if (dmc->dr.bytes == 0) {
            if (dmc->loop) {
                DMC_restart(dmc);
            } else if (dmc->irqEnabled) {
                // dmc->iFlag = TRUE;
            }
        }
    }
}

byte cnes_pAPU_DMC_DAC(cnes_pAPU_DMC_s* dmc)
{
    dmc->dac = dmc->counter;
    return dmc->dac;
}

// frame sequencer
#define FS_EVT_SI   0x4     /* set interrupt flag */
#define FS_EVT_CLC  0x2     /* clock length counters and sweep units */
#define FS_EVT_CE   0x1     /* clock envolopes and linear counter */

#define FS_IS_EVT_SI(evt)   test_bit((evt), 2)
#define FS_IS_EVT_CLC(evt)  test_bit((evt), 1)
#define FS_IS_EVT_CE(evt)   test_bit((evt), 0)

static byte FS_four_step_seq[4] = { 
    FS_EVT_CE,
    FS_EVT_CE | FS_EVT_CLC,
    FS_EVT_CE,
    FS_EVT_CE | FS_EVT_CLC | FS_EVT_SI
};

static byte FS_five_step_seq[5] = {
    FS_EVT_CE, 
    FS_EVT_CE | FS_EVT_CLC, 
    FS_EVT_CE, 
    FS_EVT_CE | FS_EVT_CLC, 
    0
};

static void FS_div_callback(cnes_pAPU_frame_seq_s* frame_seq)
{
    cnes_pAPU_seq_clock(&frame_seq->sequencer);
}

static void FS_seq_callback(cnes_pAPU_frame_seq_s* frame_seq)
{
    byte evt = cnes_pAPU_seq_value(&frame_seq->sequencer);
    // printf("evt: %d\n", evt);
    if (FS_IS_EVT_CE(evt)) {
        // printf("clock envolopes\n");
        cnes_pAPU_EG_clock(&frame_seq->sc1.eg);
        cnes_pAPU_EG_clock(&frame_seq->sc2.eg);
        cnes_pAPU_linear_clock(&frame_seq->tc.linear);
        cnes_pAPU_EG_clock(&frame_seq->nc.eg);
    }
    if (FS_IS_EVT_CLC(evt)) {
        // printf("clock length counters\n");
        cnes_pAPU_LC_clock(&frame_seq->sc1.lc);
        cnes_pAPU_LC_clock(&frame_seq->sc2.lc);
        cnes_pAPU_LC_clock(&frame_seq->tc.lc);
        cnes_pAPU_LC_clock(&frame_seq->nc.lc);
        cnes_pAPU_SU_clock(&frame_seq->sc1.su);
        cnes_pAPU_SU_clock(&frame_seq->sc2.su);
    }
    if (FS_IS_EVT_SI(evt)) {
        // printf("set interrupt flag\n");
        if (frame_seq->irqEnabled) {
            // frame_seq->iFlag = TRUE;
        }
    }
    // printf("********\n");
}

void cnes_pAPU_FS_init(cnes_pAPU_frame_seq_s* frame_seq, cnes_memory_s* memory)
{
    assert(NOTNULL(memory));
    frame_seq->memory = memory;
    frame_seq->mode = 0;        // Not sure
    frame_seq->irqEnabled = FALSE; // Not sure
    frame_seq->iFlag = FALSE;
    
    cnes_pAPU_div_init(&frame_seq->divider, 
        (cnes_pAPU_divider_callback) FS_div_callback, frame_seq);
    // *NOTE*: apu cycle = 60 * 341/3 * 261
    // set divider period = apu cycle / 240 = 7373 (approx)
    // to produce 240HZ divider clock rate
    cnes_pAPU_div_configure(&frame_seq->divider, 7373);
    cnes_pAPU_seq_init(&frame_seq->sequencer, 
        (cnes_pAPU_sequencer_callback) FS_seq_callback, frame_seq);
    cnes_pAPU_seq_configure(&frame_seq->sequencer, FS_four_step_seq, 4);
    
    cnes_pAPU_SC_init(&frame_seq->sc1, 1);
    cnes_pAPU_SC_init(&frame_seq->sc2, 2);
    cnes_pAPU_TC_init(&frame_seq->tc);
    cnes_pAPU_NC_init(&frame_seq->nc);
    cnes_pAPU_DMC_init(&frame_seq->dmc, frame_seq->memory);
}

void 
cnes_pAPU_FS_configure(cnes_pAPU_frame_seq_s* frame_seq, 
uint mode, BOOL irqEnabled)
{
    cnes_pAPU_div_reset(&frame_seq->divider);
    cnes_pAPU_seq_reset(&frame_seq->sequencer);
    frame_seq->mode = mode;
    frame_seq->irqEnabled = irqEnabled;
    if (frame_seq->mode == 0) {
        cnes_pAPU_seq_configure(&frame_seq->sequencer, 
            FS_four_step_seq, 4);
    } else if (frame_seq->mode == 1) {
        cnes_pAPU_seq_configure(&frame_seq->sequencer, 
            FS_five_step_seq, 5);
    }
    cnes_pAPU_seq_clock(&frame_seq->sequencer);
}

void cnes_pAPU_FS_clock(cnes_pAPU_frame_seq_s* frame_seq)
{
    cnes_pAPU_div_clock(&frame_seq->divider);
}

// apu
static void APU_mix_LUT_init(void);

static void APU_adjust_timing(cnes_pAPU_apu_s* apu, float fps)
{
                                        /* TODO: cpuCyclesPerScanline */
    apu->apuCyclesPerSecond = fps * 261 * 341 / 3;  // 60fps: 1780020
    apu->apuCyclesPerSample = apu->apuCyclesPerSecond / 
        apu->audio.obtained.freq;
}

error_t 
cnes_pAPU_apu_init(
cnes_pAPU_apu_s* apu, 
cnes_memory_s* memory, uint freq, uint buffers, BOOL stereo)
{
    apu->freq = freq;
    apu->buffers = buffers;
    apu->stereo = stereo;
    APU_mix_LUT_init();
    cnes_pAPU_FS_init(&apu->frame_seq, memory);
    apu->apu_cycles = 0;
    apu->apuCyclesPerSecond = 0;
    apu->apuCyclesPerSample = 0;
    apu->volume = 0.5f;
    apu->sc1_vol = 1.0f;
    apu->sc2_vol = 1.0f;
    apu->tc_vol = 1.0f;
    apu->nc_vol = 1.0f;
    apu->dmc_vol = 1.0f;
    apu->synth = S_LA;
    apu->sc1_dac = 0;
    apu->sc2_dac = 0;
    apu->tc_dac = 0;
    apu->nc_dac = 0;
    apu->dmc_dac = 0;
    apu->dac = 0;
    if (FAILED(cnes_audio_open(&apu->audio, 
        apu->freq, apu->buffers, apu->stereo))) {
        return SDL_ERROR;
    }
    cnes_audio_play(&apu->audio);
    return SUCCESS;
}

void cnes_pAPU_apu_close(cnes_pAPU_apu_s* apu)
{
    cnes_audio_pause(&apu->audio);
    cnes_audio_close(&apu->audio);
}

void cnes_pAPU_apu_dumpb(FILE* fout, const cnes_pAPU_apu_s* apu)
{
    
}

void cnes_pAPU_apu_loadb(cnes_pAPU_apu_s* apu, FILE* fin)
{
    cnes_memory_s* memory = apu->frame_seq.memory;
    uint freq = apu->freq;
    uint buffers = apu->buffers;
    BOOL stereo = apu->stereo;
    cnes_pAPU_apu_close(apu);
    cnes_pAPU_apu_init(apu, memory, freq, buffers, stereo);
}

// TODO: fps --> speed
void cnes_pAPU_apu_cycle(cnes_pAPU_apu_s* apu, uint apu_cycle, float fps)
{
    APU_adjust_timing(apu, fps);
    // printf("%d\n", apu->apuCyclesPerSample);
    for (uint i = 0; i < apu_cycle; i ++) {
        cnes_pAPU_FS_clock(&apu->frame_seq);
        cnes_pAPU_timer_clock(&apu->frame_seq.tc.timer);
        cnes_pAPU_timer_clock(&apu->frame_seq.nc.timer);
        cnes_pAPU_timer_clock(&apu->frame_seq.dmc.timer);
        // Square channels: timer with divide-by-two on the output
        if (apu->apu_cycles % 2) {  // Not sure 
            cnes_pAPU_timer_clock(&apu->frame_seq.sc1.timer);
            cnes_pAPU_timer_clock(&apu->frame_seq.sc2.timer);
        }
        if (apu->apu_cycles % apu->apuCyclesPerSample == 0) {
            if (apu->stereo) {
                // unsigned to signed
                float left = 2 * (cnes_pAPU_apu_DAC(apu, C_LEFT) - 0.5);
                float right = 2 * (cnes_pAPU_apu_DAC(apu, C_RIGHT) - 0.5);
                cnes_audio_addsample(&apu->audio, left * 0x7f);
                cnes_audio_addsample(&apu->audio, right * 0x7f);
            } else {
                // unsigned to signed
                float mono = 2 * (cnes_pAPU_apu_DAC(apu, C_MONO) - 0.5);
                cnes_audio_addsample(&apu->audio, mono * 0x7f);;
            }
        }
        apu->apu_cycles += 1;
        if (apu->apu_cycles >= 1e6 * 341) {
            apu->apu_cycles = 0;
        }
    }
}

#include "2A03.h"

static byte APU_status_read(cnes_pAPU_apu_s* apu)
{
    byte status = 0;
    if (apu->frame_seq.dmc.iFlag) {
        set_bit(&status, 7);
    }
    if (apu->frame_seq.iFlag) {
        set_bit(&status, 6);
    }
    if (apu->frame_seq.dmc.dr.bytes > 0) {
        set_bit(&status, 4);
    }
    if (apu->frame_seq.nc.lc.counter > 0) {
        set_bit(&status, 3);
    }
    if (apu->frame_seq.tc.lc.counter > 0) {
        set_bit(&status, 2);
    }
    if (apu->frame_seq.sc2.lc.counter > 0) {
        set_bit(&status, 1);
    }
    if (apu->frame_seq.sc1.lc.counter > 0) {
        set_bit(&status, 0);
    }
    apu->frame_seq.iFlag = FALSE;
    return status;
}

static void APU_status_write(cnes_pAPU_apu_s* apu, byte value)
{
    if (test_bit(value, 4)) {
        if (apu->frame_seq.dmc.dr.bytes <= 0) {
            DMC_restart(&apu->frame_seq.dmc);
        }
    } else {
        apu->frame_seq.dmc.dr.bytes = 0;
    }
    apu->frame_seq.dmc.iFlag = FALSE;
    cnes_pAPU_LC_enable(&apu->frame_seq.nc.lc, test_bit(value, 3));
    cnes_pAPU_LC_enable(&apu->frame_seq.tc.lc, test_bit(value, 2));
    cnes_pAPU_LC_enable(&apu->frame_seq.sc2.lc, test_bit(value, 1));
    cnes_pAPU_LC_enable(&apu->frame_seq.sc1.lc, test_bit(value, 0));
}

byte cnes_pAPU_apu_read(cnes_pAPU_apu_s* apu, word addr)
{
    if (addr == CNES_2A03_APU_SIG) {
        return APU_status_read(apu);
    }
    else {
        return 0; // Illegal access
    }
}

void cnes_pAPU_apu_write(cnes_pAPU_apu_s* apu, word addr, byte value)
{
    switch (addr) {
        case CNES_2A03_APU_P1C:
        case CNES_2A03_APU_P1RC:
        case CNES_2A03_APU_P1FT:
        case CNES_2A03_APU_P1CT:
            cnes_pAPU_SC_write(&apu->frame_seq.sc1, 
                addr - CNES_2A03_APU_P1C, value);
            break;
        case CNES_2A03_APU_P2C:
        case CNES_2A03_APU_P2RC:
        case CNES_2A03_APU_P2FT:
        case CNES_2A03_APU_P2CT:
            cnes_pAPU_SC_write(&apu->frame_seq.sc2, 
                addr - CNES_2A03_APU_P2C, value);
            break;
        case CNES_2A03_APU_TRIC1:
        case CNES_2A03_APU_TRIF1:
        case CNES_2A03_APU_TRIF2:
            cnes_pAPU_TC_write(&apu->frame_seq.tc, 
                addr - CNES_2A03_APU_TRIC1, value);
            break;
        case CNES_2A03_APU_NOIC1:
        case CNES_2A03_APU_NOIF1:
        case CNES_2A03_APU_NOIF2:
            cnes_pAPU_NC_write(&apu->frame_seq.nc, 
                addr - CNES_2A03_APU_NOIC1, value);
            break;
        case CNES_2A03_APU_DMC:
        case CNES_2A03_APU_DMDA:
        case CNES_2A03_APU_DMA:
        case CNES_2A03_APU_MODDL:
            cnes_pAPU_DMC_write(&apu->frame_seq.dmc, 
                addr - CNES_2A03_APU_DMC, value);
            break;
        case CNES_2A03_APU_SIG:
            APU_status_write(apu, value);
            break;
        case CNES_2A03_PORT2: {
            uint mode = test_bit(value, 7) >> 7;
            BOOL irqEnabled = test_bit(value, 6);
            cnes_pAPU_FS_configure(&apu->frame_seq, mode, irqEnabled);
            break;
        }
        default:
            // Illegal access
            break;
    }
}

void cnes_pAPU_apu_watch(cnes_pAPU_apu_s* apu, cnes_vdb_s* vdb)
{
    cnes_vdb_watch_var(vdb, &apu->frame_seq.sc1.dac, 
        T_U32, 15, C_RED, 0xff, NULL);
    cnes_vdb_watch_var(vdb, &apu->frame_seq.sc2.dac, 
        T_U32, 15, C_GREEN, 0xff, NULL);
    cnes_vdb_watch_var(vdb, &apu->frame_seq.tc.linear.counter, 
        T_U32, 0x7f, C_BLUE, 0xff, NULL);
    cnes_vdb_watch_var(vdb, &apu->frame_seq.nc.lc.counter, 
        T_U32, 15, C_IVORY, 0xff, NULL);
    cnes_vdb_watch_var(vdb, &apu->frame_seq.sc1.timer.period, 
        T_U32, 2<<11, C_ORANGE, 0xff, NULL);
    cnes_vdb_watch_var(vdb, &apu->frame_seq.sc1.timer.counter, 
        T_U32, 2<<11, C_VOILET, 0xff, NULL);
    cnes_vdb_watch_var(vdb, &apu->audio.buffer_pos, 
        T_U32, apu->audio.buf_size, C_PINK, 0xff, NULL);
}

static void APU_DAC_collect(cnes_pAPU_apu_s* apu, cnes_pAPU_apu_channel_t channel)
{
    if (apu->stereo) {
        apu->sc1_dac = apu->intensity * apu->sc1_vol * 
            cnes_pAPU_SC_DAC(&apu->frame_seq.sc1);
        apu->sc2_dac = apu->intensity * apu->sc2_vol * 
            cnes_pAPU_SC_DAC(&apu->frame_seq.sc2);
    }
    if (channel == C_MONO || channel == C_LEFT) {
        apu->sc1_dac = apu->sc1_vol * cnes_pAPU_SC_DAC(&apu->frame_seq.sc1);
    }
    if (channel == C_MONO || channel == C_RIGHT) {
        apu->sc2_dac= apu->sc2_vol * cnes_pAPU_SC_DAC(&apu->frame_seq.sc2);
    }
    apu->tc_dac = apu->tc_vol * cnes_pAPU_TC_DAC(&apu->frame_seq.tc);
    apu->nc_dac= apu->nc_vol * cnes_pAPU_NC_DAC(&apu->frame_seq.nc);
    apu->dmc_dac = apu->dmc_vol * cnes_pAPU_DMC_DAC(&apu->frame_seq.dmc);
}

static float APU_mix_formulas(const cnes_pAPU_apu_s* apu)
{
    float sub_denominator = apu->sc1_dac + apu->sc2_dac;
    float square_out = sub_denominator == 0 ? 0 :
                        (95.88 / (8128.0 /sub_denominator + 100));
    sub_denominator = apu->tc_dac / 8227.0 + apu->nc_dac / 12241.0 + 
        apu->dmc_dac / 22638.0;
    float tnd_out = sub_denominator == 0 ? 0:
                        (159.79 / (1.0 / sub_denominator + 100));
    // [0.0, 1.0]
    return square_out + tnd_out;
}

static float APU_mix_LUT_square[31];
static float APU_mix_LUT_tnd[203];

static void APU_mix_LUT_init(void)
{
    for (sint i = 0; i < 31; i ++) {
        APU_mix_LUT_square[i] = 95.52 / (8128.0 / i + 100);
    }
    for (sint i = 0; i < 203; i ++) {
        APU_mix_LUT_tnd[i] = 163.67 / (24329.0 / i + 100);
    }
}

static float APU_mix_LUT(const cnes_pAPU_apu_s* apu)
{
    float square_out = APU_mix_LUT_square[apu->sc1_dac + apu->sc2_dac];
    float tnd_out = APU_mix_LUT_tnd[3 * apu->tc_dac + 2 * apu->nc_dac + 
        apu->dmc_dac];
    return square_out + tnd_out;
}

static float APU_mix_LA(const cnes_pAPU_apu_s* apu)
{
    float square_out = 0.00752 * (apu->sc1_dac + apu->sc2_dac);
    float tnd_out = 0.00851 * apu->tc_dac + 0.00494 * apu->nc_dac + 
        0.00335 * apu->dmc_dac;
    return square_out + tnd_out;
}

float cnes_pAPU_apu_DAC(cnes_pAPU_apu_s* apu, cnes_pAPU_apu_channel_t channel)
{
    apu->dac = .0f;
    APU_DAC_collect(apu, channel);
    switch (apu->synth) {
        case S_FORMULAS:
            apu->dac = APU_mix_formulas(apu);
            break;
        case S_LA:
            apu->dac = APU_mix_LA(apu);
            break;
        case S_LUT:
            apu->dac = APU_mix_LUT(apu);
            break;
        default:
            break;
    }
    assert(BETWEEN(apu->dac, .0f, 1.0f));
    apu->dac *= apu->volume;
    assert(BETWEEN(apu->dac, .0f, 1.0f));
    return apu->dac;
}