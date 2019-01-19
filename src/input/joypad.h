// joypad.h
// Author: hyan23
// Date: 2018.05.11

#ifndef __JOYPAD_H__
#define __JOYPAD_H__

#include <SDL/SDL.h>
#include "header.h"
#include "input.h"

extern uint cnes_joypad_indexes1[4];

#define CNES_JOYSTICK_GET_BUTTON(joystick, button)  \
    (SDL_JoystickGetButton((joystick), (button)) ?  \
        CNES_INPUT_KEYDOWN : CNES_INPUT_KEYUP)

typedef struct CNES_JOYSTICK
{
    SDL_Joystick*   joystick;
    uint            index;
    const char*     name;
    uint            numAxes;
    uint            numButtons;
    // TODO: configurable
    uint            indexes[4]; // button indexes. A:B:SELECT:START
    bit*            states;     // button states
}
cnes_joypad_s;

extern error_t cnes_joypad_init(cnes_joypad_s* joypad, 
    SDL_Joystick* joystick, const uint (*indexes)[4]);
extern void cnes_joypad_close(cnes_joypad_s* joypad);
extern void cnes_joypad_dump(const cnes_joypad_s* joypad);
extern BOOL cnes_joypad_ack(const cnes_joypad_s* joypad);
extern void cnes_joypad_read(cnes_joypad_s* joypad, 
    cnes_input_s* input, cnes_input_port_t port, cnes_input_status_t status);

#endif /* __JOYPAD_H__ */