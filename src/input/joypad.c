// joypad.c
// Author: hyan23
// Date: 2018.05.11

#include "joypad.h"

uint cnes_joypad_indexes1[4] = {
    1, 0, 8, 9
};

static void joypad_assert_indexes(const cnes_joypad_s* joypad)
{
    for (sint i = 0; i < 4; i ++) {
        assert(joypad->indexes[i] < joypad->numButtons);
    }
}

error_t cnes_joypad_init(cnes_joypad_s* joypad, 
    SDL_Joystick* joystick, const uint (*indexes)[4]) {
    CLEAR(joypad);
    joypad->joystick = joystick;
    joypad->index = SDL_JoystickIndex(joypad->joystick);
    joypad->name = SDL_JoystickName(joypad->index);
    joypad->numAxes = SDL_JoystickNumAxes(joypad->joystick);
    joypad->numButtons = SDL_JoystickNumButtons(joypad->joystick);
    assert(joypad->numAxes >= 2);
    assert(joypad->numButtons >= 4);
    memcpy(joypad->indexes, indexes, sizeof (joypad->indexes));
    joypad_assert_indexes(joypad);
    joypad->states = MALLOC(bit, joypad->numButtons);
    if (NUL(joypad->states)) {
        return FAILURE;
    }
    return SUCCESS;
}

void cnes_joypad_close(cnes_joypad_s* joypad)
{
    if (NOTNULL(joypad->joystick)) {
        if (SDL_JoystickOpened(joypad->index)) {
            SDL_JoystickClose(joypad->joystick);
        }
    }
    FREE(joypad->states);
}

void cnes_joypad_dump(const cnes_joypad_s* joypad)
{
    printf("joypad#%d: %s\n", joypad->index, joypad->name);
    printf("number axes: %u\n", joypad->numAxes);
    printf("nubmer buttons: %u\n", joypad->numButtons);
    printf("button indexes %u:%u:%u:%u\n", 
        joypad->indexes[0], joypad->indexes[1], 
        joypad->indexes[2], joypad->indexes[3]);
}

BOOL cnes_joypad_ack(const cnes_joypad_s* joypad)
{
    return NOTNULL(SDL_JoystickName(joypad->index));
}

static void joypad_read_states(cnes_joypad_s* joypad)
{
    for (uint i = 0; i < joypad->numButtons; i ++) {
        joypad->states[i] = CNES_JOYSTICK_GET_BUTTON(joypad->joystick, i);
    }
}

static void 
joypad_read_buttons(const cnes_joypad_s* joypad, 
cnes_input_s* input, cnes_input_port_t port, cnes_input_status_t status) 
{
    cnes_input_update(input, port, status, 
        A, joypad->states[joypad->indexes[0]]);
    cnes_input_update(input, port, status, 
        B, joypad->states[joypad->indexes[1]]);
    cnes_input_update(input, port, status, 
        SELECT, joypad->states[joypad->indexes[2]]);
    cnes_input_update(input, port, status, 
        START, joypad->states[joypad->indexes[3]]);
}

static void 
joypad_read_axis(cnes_joypad_s* joypad, 
cnes_input_s* input, cnes_input_port_t port, cnes_input_status_t status) 
{
    Sint16 value = SDL_JoystickGetAxis(joypad->joystick, 0);
    if (value < 0) {
        cnes_input_update(input, port, status, 
            LEFT, CNES_INPUT_KEYDOWN);
    } else if (value > 0) {
        cnes_input_update(input, port, status, 
            RIGHT, CNES_INPUT_KEYDOWN);
    } else {
        cnes_input_update(input, port, status, 
            LEFT, CNES_INPUT_KEYUP);
        cnes_input_update(input, port, status, 
            RIGHT, CNES_INPUT_KEYUP);
    }
    value = SDL_JoystickGetAxis(joypad->joystick, 1);
    if (value < 0) {
        cnes_input_update(input, port, status, 
            UP, CNES_INPUT_KEYDOWN);
    } else if (value > 0) {
        cnes_input_update(input, port, status, 
            DOWN, CNES_INPUT_KEYDOWN);
    } else {
        cnes_input_update(input, port, status, 
            UP, CNES_INPUT_KEYUP);
        cnes_input_update(input, port, status, 
            DOWN, CNES_INPUT_KEYUP);
    }
}

void 
cnes_joypad_read(cnes_joypad_s* joypad, 
cnes_input_s* input, cnes_input_port_t port, cnes_input_status_t status) 
{
    SDL_JoystickUpdate();
    joypad_read_states(joypad);
    joypad_read_buttons(joypad, input, port, status);
    joypad_read_axis(joypad, input, port, status);
}