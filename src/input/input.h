// input.h
// Author: hyan23
// Date: 2018.04.02

#ifndef __INPUT_H__
#define __INPUT_H__

#include "header.h"

#define CNES_INPUT_KEYUP            0
#define CNES_INPUT_KEYDOWN          1

#define CNES_INPUT_JP1_CONNECTED    0x1
#define CNES_INPUT_JP2_CONNECTED    0x2

typedef enum CNES_INPUT_PORT {
    PORT1,  PORT2
} cnes_input_port_t;

// Each port can connect up to 2 joypads
typedef enum CNES_INPUT_STATUS {
    MASTER, SLAVE
} cnes_input_status_t;

typedef enum CNES_INPUT_BUTTON {
    A,  B, 
    SELECT, START, 
    UP,     DOWN,   LEFT,   RIGHT
} cnes_input_button_t;

typedef enum CNES_INPUT_SLOT {
    A1,         B1, 
    SELECT1,    START1, 
    UP1,        DOWN1,      LEFT1,      RIGHT1, 
    A2,         B2, 
    SELECT2,    START2, 
    UP2,        DOWN2,      LEFT2,      RIGHT2, 
    SIG0,       SIG1,       SIG2,       SIG3, 
    UNUSED0,    UNUSED1,    UNUSED2,    UNUSED3
} cnes_input_slot_t;

typedef struct CNES_INPUT
{
    byte                slot[2][24];
    cnes_input_slot_t   shift[2];
    byte                latch;
    bit                 RESET;  // emulator use
    bit                 QUIT;
    bit                 SAVE;
    bit                 LOAD;
}
cnes_input_s;

extern void cnes_input_init(cnes_input_s* input);
extern void cnes_input_reset(cnes_input_s* input);
extern byte cnes_input_read(cnes_input_s* input, cnes_input_port_t port);
extern void cnes_input_write(cnes_input_s* input, byte value);
extern void cnes_input_update(cnes_input_s* input, 
    cnes_input_port_t port, cnes_input_status_t status, 
    cnes_input_button_t button, byte state);

#endif /* __INPUT_H__ */