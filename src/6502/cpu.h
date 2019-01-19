// cpu.h
// Author: hyan23
// Date: 2017.08.15

#ifndef __CPU_H__
#define __CPU_H__

#include "header.h"
#include "6502.h"
#include "memory.h"

typedef enum CNES_INTERRUPT {
    INT_NMI, 
    INT_RESET, 
    INT_BRK, 
    INT_IRQ, 
    INT_NONE
}
cnes_interrupt_t;

typedef struct CNES_CPU
{
    struct {
        byte                accumulator;
        byte                x_index_register;
        byte                y_index_register;
        byte                status_register;
        word                program_counter;
        byte                stack_pointer;
        cnes_interrupt_t    interrupt;
    };
    cnes_memory_s*  memory;
    byte            opcode;
    word            operand;
    word            addr;
    byte            src;
    BOOL            BC;     /* boundary crossed */
    word            temp;
    sint            cycles;
}
cnes_cpu_s;

extern void cnes_cpu_init(cnes_cpu_s* cpu, cnes_memory_s* memory);
extern void cnes_cpu_reset(cnes_cpu_s* cpu);
extern uint cnes_cpu_step(cnes_cpu_s* cpu, BOOL dump);
extern void cnes_cpu_cycle(cnes_cpu_s* cpu, sint cycles, BOOL dump);
extern void cnes_cpu_dump(const cnes_cpu_s* cpu);
extern void cnes_cpu_dumpb(FILE* fout, const cnes_cpu_s* cpu);
extern void cnes_cpu_loadb(cnes_cpu_s* cpu, FILE* fin);
extern void cnes_cpu_signal(cnes_cpu_s* cpu, cnes_interrupt_t interrupt);

#endif /* __CPU_H__ */