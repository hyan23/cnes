// 6502.c
// Author: hyan23
// Date: 2017.08.03

#include "header.h"
#include "6502.h"

const char* CNES_ADDRESSING_STR[CNES_ADDRESSING_NUMBER] = {
    "M_ACCUMULATOR",    "M_IMMEDIATE",  "M_IMPLIED", 
    "M_ZEROPAGE",   "M_ABSOLUTE", 
    "M_XINDEXED_ZEROPAGE",  "M_YINDEXED_ZEROPAGE",  "M_XINDEXED",   "M_YINDEXED", 
    "M_INDIRECT",   "M_INDEXED_INDIRECT",   "M_INDIRET_INDEXED", 
    "M_RELATIVE"
};

void cnes_dump_instruction(word program_counter, byte opcode, word operand)
{
    assert(!CNES_BAD_OPCODE(opcode));
    printf("0x%04x\t", program_counter);
    const cnes_instruction_s* instruction = &instructions[opcode];
    printf("%s\t", instruction->name);
    if (instruction->addressing != M_IMPLIED) {
        const char* form = instruction->form;
        while (*form) {
            if (*form == '?') {
                printf("$%04x", operand);
            } else if (*form == '&') {
                printf("$%02x", operand);
            } else {
                putchar(*form);
            }
            form ++;
        }
    }
    putchar('\n');
}