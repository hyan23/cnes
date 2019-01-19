// keyboard.c
// Author: hyan23
// Date: 2018.05.11

#include <SDL/SDL.h>
#include "header.h"
#include "keyboard.h"

// TODO: modifier+key
static SDLKey keyboard_indexes1[CNES_KEYBOARD_KEYS] = {
    SDLK_SPACE,     SDLK_LSHIFT, 
    SDLK_q,         SDLK_e, 
    SDLK_w, SDLK_s, SDLK_a, SDLK_d, 
    SDLK_F1,    SDLK_F2
};

static SDLKey keyboard_indexes2[CNES_KEYBOARD_KEYS] = {
    SDLK_j,         SDLK_k, 
    SDLK_RSHIFT,    SDLK_RETURN, 
    SDLK_UP,    SDLK_DOWN,  SDLK_LEFT,  SDLK_RIGHT, 
    SDLK_UNKNOWN,   SDLK_UNKNOWN
};

error_t cnes_keyboard_init(cnes_keyboard_s* keyboard, uint index) {
    if (index >= CNES_KEYBOARD_NUMBER) {
        return FAILURE;
    }
    CLEAR(keyboard);
    switch (index) {
        case 0:
            memcpy(keyboard->indexes, keyboard_indexes1, 
                sizeof (keyboard->indexes));
            break;
        case 1:
            memcpy(keyboard->indexes, keyboard_indexes2, 
                sizeof (keyboard->indexes));
            break;
        default:
            break;
    }
    keyboard->index = index;
    return SUCCESS;
}

static void 
keyboard_read_buttons(const cnes_keyboard_s* keyboard, 
SDLKey sym, byte state, 
cnes_input_s* input, cnes_input_port_t port, cnes_input_status_t status) 
{
    if (sym == keyboard->indexes[0]) {
        cnes_input_update(input, port, status, A, state);
    } else if (sym == keyboard->indexes[1]) {
        cnes_input_update(input, port, status, B, state);
    } else if (sym == keyboard->indexes[2]) {
        cnes_input_update(input, port, status, SELECT, state);
    } else if (sym == keyboard->indexes[3]) {
        cnes_input_update(input, port, status, START, state);
    } else if (sym == keyboard->indexes[4]) {
        cnes_input_update(input, port, status, UP, state);
    } else if (sym == keyboard->indexes[5]) {
        cnes_input_update(input, port, status, DOWN, state);
    } else if (sym == keyboard->indexes[6]) {
        cnes_input_update(input, port, status, LEFT, state);
    } else if (sym == keyboard->indexes[7]) {
        cnes_input_update(input, port, status, RIGHT, state);
    } else if (sym == keyboard->indexes[8]) {
        input->SAVE = state;
    } else if (sym == keyboard->indexes[9]) {
        input->LOAD = state;
    }
}

SDL_Event events[CNES_KEYBOARD_EVENTS];
static sint numEvents = 0;

void 
cnes_keyboard_read(const cnes_keyboard_s* keyboard, 
cnes_input_s* input, cnes_input_port_t port, cnes_input_status_t status)
{
    if (keyboard->index == 0) {
        SDL_PumpEvents();
        // call cnes_keyboard_flush to flush these
        numEvents = SDL_PeepEvents(events, 
            CNES_KEYBOARD_EVENTS, SDL_PEEKEVENT, CNES_KEYBOARD_MASKS);
    }
    for (sint i = 0; i < numEvents; i ++) {
        SDL_Event* event = &events[i];
        switch (event->type) {
            case SDL_QUIT:
                input->QUIT = CNES_INPUT_KEYDOWN;
                break;
            case SDL_KEYUP:
            case SDL_KEYDOWN:
                keyboard_read_buttons(keyboard, event->key.keysym.sym, 
                    event->type == SDL_KEYDOWN ? CNES_INPUT_KEYDOWN : 
                    CNES_INPUT_KEYUP, 
                    input, port, status);
                break;
            default:
                break;
        }
    }
}

void cnes_keyboard_flush(void)
{
    SDL_PeepEvents(events, numEvents, SDL_GETEVENT, CNES_KEYBOARD_MASKS);
    SDL_PeepEvents(events, CNES_KEYBOARD_EVENTS, SDL_GETEVENT, 
        ~CNES_KEYBOARD_EVENTS);
    CLEAR(&events);
    numEvents = 0;
}