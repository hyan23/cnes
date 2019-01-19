// keyboard.h
// Author: hyan23
// Date: 2018.05.10

#ifndef __KEYBOARD_H__
#define __KEYBOARD_H__

#include <SDL/SDL.h>
#include "header.h"
#include "input.h"

#define CNES_KEYBOARD_KEYS          10

#define CNES_KEYBOARD_NUMBER        2
#define CNES_KEYBOARD_EVENTS        10

#define CNES_KEYBOARD_MASKS (SDL_QUITMASK | SDL_KEYUPMASK | SDL_KEYDOWNMASK)

typedef struct CNES_KEYBOARD
{
    // A:B:SELECT:START:UP:DOWN:LEFT:RIGHT
    // SAVE:LOAD
    SDLKey  indexes[CNES_KEYBOARD_KEYS];    // TODO: configurable
    uint    index;      // keyboard index
}
cnes_keyboard_s;

extern error_t cnes_keyboard_init(cnes_keyboard_s* keyboard, uint index);
extern void cnes_keyboard_read(const cnes_keyboard_s* keyboard, 
    cnes_input_s* input, cnes_input_port_t port, cnes_input_status_t status);
extern void cnes_keyboard_flush(void);

#endif /* __KEYBOARD_H__ */