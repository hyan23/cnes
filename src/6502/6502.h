// 6502.h
// Author: hyan23
// Date: 2017.08.02

#ifndef __6502_H__
#define __6502_H__

#include "header.h"

typedef enum CNES_ADDRESSING {
    M_ACCUMULATOR,  M_IMMEDIATE,    M_IMPLIED, 
    M_ZEROPAGE, M_ABSOLUTE,
    M_XINDEXED_ZEROPAGE,    M_YINDEXED_ZEROPAGE,    M_XINDEXED, M_YINDEXED, 
    M_INDIRECT, M_INDEXED_INDIRECT, M_INDIRET_INDEXED, 
    M_RELATIVE
}
cnes_addressing_t;

#define CNES_ADDRESSING_NUMBER      13
const char* CNES_ADDRESSING_STR[CNES_ADDRESSING_NUMBER];

typedef struct CNES_INSTRUCTION
{
    const char*         name;
    const char*         form;
    byte                opcode;
    byte                bytes;
    uint                cycles;
    cnes_addressing_t   addressing;
}
cnes_instruction_s;

#define CNES_INSTRUCTION_NUMBER     0x100
#define CNES_BAD_OPCODE(opcode)     NUL(instructions[opcode].name)

extern const cnes_instruction_s instructions[CNES_INSTRUCTION_NUMBER];

extern void cnes_dump_instruction(word program_counter, 
    byte opcode, word operand);

#endif /* __6502_H__ */