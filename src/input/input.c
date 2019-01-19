// input.c
// Author: hyan23
// Date: 2018.04.02

#include "input.h"

void cnes_input_init(cnes_input_s* input)
{
    CLEAR(input);
    cnes_input_reset(input);
}

void cnes_input_reset(cnes_input_s* input)
{
    memset(input->slot, CNES_INPUT_KEYUP, sizeof (input->slot));
    // Not sure
    input->slot[PORT1][SIG0] = input->slot[PORT1][SIG1] = 
        input->slot[PORT1][SIG2] = input->slot[PORT1][SIG3] = 
        CNES_INPUT_JP1_CONNECTED;
    // Not sure
    input->slot[PORT2][SIG0] = input->slot[PORT2][SIG1] = 
        input->slot[PORT2][SIG2] = input->slot[PORT2][SIG3] = 
        CNES_INPUT_JP2_CONNECTED;
}

#define ASSERT_SLOT(slot)   assert((slot) <= UNUSED3)
#define ASSERT_PORT(port)   assert(((port) == PORT1) || \
                        ((port) == PORT2))

byte cnes_input_read(cnes_input_s* input, cnes_input_port_t port)
{
    ASSERT_PORT(port);
    ASSERT_SLOT(input->shift[port]);
    byte value = input->slot[port][input->shift[port]];
    if (input->shift[port] < UNUSED3) {
        input->shift[port] ++;
    } else {
        // Not sure
        input->shift[port] = A1;
    }
    return value;
}

void cnes_input_write(cnes_input_s* input, byte value)
{
    // Write a 1 followed a 0 on the lowest bit to reset shift registers
    input->latch = test_bit(value, 0) ? 1 : 0;
    if (input->latch == 0) {
        input->shift[PORT1] = A1;
        input->shift[PORT2] = A1;
    }
}

#define SLOT_NAME(symbol, id)   symbol##id
#define SLOT_MASTER(symbol)     SLOT_NAME(symbol, 1)
#define SLOT_SLAVE(symbol)      SLOT_NAME(symbol, 2)

#define MAP_SLOT(status, symbol)                \
    (status == MASTER ? SLOT_MASTER(symbol) :   \
    SLOT_SLAVE(symbol))

void cnes_input_update(cnes_input_s* input, 
cnes_input_port_t port, cnes_input_status_t status, 
cnes_input_button_t button, byte state) {
    ASSERT_PORT(port);
    assert(status == MASTER || status == SLAVE);
    cnes_input_slot_t slot = A1;
    switch (button) {
        case A:
            slot = MAP_SLOT(status, A);
            break;
        case B:
            slot = MAP_SLOT(status, B);
            break;
        case SELECT:
            slot = MAP_SLOT(status, SELECT);
            break;
        case START:
            slot = MAP_SLOT(status, START);
            break;
        case UP:
            slot = MAP_SLOT(status, UP);
            break;
        case DOWN:
            slot = MAP_SLOT(status, DOWN);
            break;
        case LEFT:
            slot = MAP_SLOT(status, LEFT);
            break;
        case RIGHT:
            slot = MAP_SLOT(status, RIGHT);
            break;
        default:
            assert(FALSE);
    }
    input->slot[port][slot] = state;
    input->slot[port][slot] = state;
}