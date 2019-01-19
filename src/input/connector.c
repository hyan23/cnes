// connector.c
// Author: hyan23
// Date: 2018.05.11

#include <SDL/SDL.h>
#include "header.h"
#include "joypad.h"
#include "keyboard.h"
#include "zapper.h"
#include "connector.h"

// TODO: Hot swapping

static void detect_joypads(cnes_connector_s* connector)
{
    uint nJoysticks = SDL_NumJoysticks();
    connector->nPeripherals += nJoysticks;
    for (uint i = 0; i < nJoysticks; i ++) {
        if (connector->inUse >= CNES_CONNECTOR_PORTS)
            break;
        if (!SDL_JoystickOpened(i)) {
            SDL_Joystick* joystick = SDL_JoystickOpen(i);
            if (NOTNULL(joystick)) {
                cnes_peripheral_s* peripheral = 
                    &connector->peripherals[connector->inUse];
                if (SUCCEED(cnes_joypad_init(&peripheral->joypad, 
                    joystick, &cnes_joypad_indexes1))) {
                    peripheral->type = JOYPAD;
                    peripheral->plugged = TRUE;
                    connector->inUse ++;
                }
            }
        }
    }
}

static void detect_keyboards(cnes_connector_s* connector)
{
    connector->nPeripherals += CNES_KEYBOARD_NUMBER;
    for (sint i = 0; i < CNES_KEYBOARD_NUMBER; i ++) {
        if (connector->inUse >= CNES_CONNECTOR_PORTS)
            break;
        cnes_peripheral_s* peripheral = 
            &connector->peripherals[connector->inUse];
        if (SUCCEED(cnes_keyboard_init(&peripheral->keyboard, i))) {
            peripheral->type = KEYBOARD;
            peripheral->plugged = TRUE;
            connector->inUse ++;
        }
    }
}

static void detect_zappers(cnes_connector_s* connector)
{
    // TODO: 
}

void cnes_connector_init(cnes_connector_s* connector)
{
    CLEAR(connector);
    if (SDL_InitSubSystem(SDL_INIT_JOYSTICK) != -1) {
        detect_joypads(connector);
    }
    SDL_EnableKeyRepeat(0, 0);
    detect_keyboards(connector);
    detect_zappers(connector);
}

void cnes_connector_close(cnes_connector_s* connector)
{
    for (sint i = 0; i < CNES_CONNECTOR_PORTS; i ++) {
        cnes_peripheral_s* peripheral = &connector->peripherals[i];
        if (peripheral->plugged) {
            switch (peripheral->type) {
                case JOYPAD:
                    cnes_joypad_close(&peripheral->joypad);
                    break;
                case KEYBOARD:
                    break;
                case ZAPPER:
                    // TODO:
                    break;
                default:
                    break;
            }
        }
    }
    if (SDL_WasInit(SDL_INIT_JOYSTICK)) {
        SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
    }
}

void cnes_connector_dump(const cnes_connector_s* connector)
{
    printf("input: %u peripheral(s) plugged. %u in use\n", 
        connector->nPeripherals, connector->inUse);
    for (sint i = 0; i < CNES_CONNECTOR_PORTS; i ++) {
        const cnes_peripheral_s* peripheral = &connector->peripherals[i];
        if (peripheral->plugged) {
            switch (peripheral->type) {
                case JOYPAD:
                    cnes_joypad_dump(&peripheral->joypad);
                    break;
                case KEYBOARD:
                    break;
                case ZAPPER:
                    // TODO:
                    break;
                default:
                    break;
            }
        }
    }
}

static void connector_ack(cnes_connector_s* connector)
{
    for (sint i = 0; i < CNES_CONNECTOR_PORTS; i ++) {
        cnes_peripheral_s* peripheral = &connector->peripherals[i];
        if (peripheral->plugged) {
            switch (peripheral->type) {
                case JOYPAD:
                    // assert(cnes_joypad_ack(&peripheral->joypad));
                    assert(TRUE);
                    break;
                case KEYBOARD:
                    assert(TRUE);
                    break;
                case ZAPPER:
                    // TODO: 
                    break;
            }
        }
    }
}

void cnes_connector_read(cnes_connector_s* connector, cnes_input_s* input)
{
    connector_ack(connector);
    for (sint i = 0; i < CNES_CONNECTOR_PORTS; i ++) {
        cnes_peripheral_s* peripheral = &connector->peripherals[i];
        if (peripheral->plugged) {
            cnes_input_port_t port = i % 2 == 0 ? PORT1 : PORT2;
            cnes_input_status_t status = i < 2 ? MASTER : SLAVE;
            switch (peripheral->type) {
                case JOYPAD:
                    cnes_joypad_read(&peripheral->joypad, 
                        input, port, status);
                    break;
                case KEYBOARD:
                    cnes_keyboard_read(&peripheral->keyboard, 
                        input, port, status);
                    break;
                case ZAPPER:
                    // TODO: 
                    break;
                default:
                    break;
            }
        }
    }
    cnes_keyboard_flush();
}