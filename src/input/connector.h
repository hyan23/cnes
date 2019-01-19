// connector.h
// Author: hyan23
// Date: 2018.05.11

#ifndef __CONNECTOR_H__
#define __CONNECTOR_H__

#include <stdio.h>
#include "header.h"
#include "input.h"
#include "joypad.h"
#include "keyboard.h"
#include "zapper.h"

#define CNES_CONNECTOR_PORTS        4

typedef enum {
    JOYPAD, KEYBOARD,   ZAPPER
} cnes_peripheral_t;

typedef struct CNES_PERIPHERAL
{
    cnes_peripheral_t   type;
    BOOL                plugged;
    union {
        cnes_joypad_s       joypad;
        cnes_keyboard_s     keyboard;
        cnes_zapper_s       zapper;
    };
}
cnes_peripheral_s;

typedef struct CNES_CONNECTOR
{
    cnes_peripheral_s   peripherals[CNES_CONNECTOR_PORTS];
    uint                nPeripherals;
    uint                inUse;
}
cnes_connector_s;

extern void cnes_connector_init(cnes_connector_s* connector);
extern void cnes_connector_close(cnes_connector_s* connector);
extern void cnes_connector_dump(const cnes_connector_s* connector);
extern void cnes_connector_read(cnes_connector_s* connector, cnes_input_s* input);

#endif /* __CONNECTOR_H__ */