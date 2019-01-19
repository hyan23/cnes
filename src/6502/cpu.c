// cpu.c
// Author: hyan23
// Date: 2017.08.15

#include <stdint.h>
#include "header.h"
#include "6502.h"
#include "memory.h"
#include "cpu.h"

#define CARRYBIT        0
#define ZEROBIT         1
#define INTERRUPTBIT    2
#define DECIMALBIT      3
#define BREAKBIT        4
#define OVERFLOWBIT     6
#define SIGNBIT         7

typedef int8_t              displacement_t;
#define STACK_BASE          0x100
#define NMI_HANDLER         0xfffa
#define RESET_HANDLER       0xfffc
#define IRQ_HANDLER         0xfffe

#define AC      (cpu->accumulator)
#define XR      (cpu->x_index_register)
#define YR      (cpu->y_index_register)
#define SR      (cpu->status_register)
#define PC      (cpu->program_counter)
#define SP      (cpu->stack_pointer)

#define M           (cpu->memory)
#define OPCODE      (cpu->opcode)
#define OPERAND     (cpu->operand)
#define ADDR        (cpu->addr)
#define SRC         (cpu->src)
#define TEMP        (cpu->temp)
#define CYCLES      (cpu->cycles)

#define ADDRESSING  (instructions[OPCODE].addressing)

#define CARRY       test_bit(SR, CARRYBIT)
#define ZERO        test_bit(SR, ZEROBIT)
#define INTERRUPT   test_bit(SR, INTERRUPTBIT)
#define DECIMAL     test_bit(SR, DECIMALBIT)
#define BREAK       test_bit(SR, BREAKBIT)
#define OVERFLOW    test_bit(SR, OVERFLOWBIT)
#define SIGN        test_bit(SR, SIGNBIT)

#define SET_SIGN(temp)  (test_bit((temp), 7) ? \
        set_bit(&SR, SIGNBIT) : reset_bit(&SR, SIGNBIT))
#define SET_ZERO(temp)  (!(temp) ? \
        set_bit(&SR, ZEROBIT) : reset_bit(&SR, ZEROBIT))
#define SET_CONDITION(condition, which) ((condition) ?  \
        set_bit(&SR, (which)) : reset_bit(&SR, (which)))
#define SET_CARRY(condition)        SET_CONDITION(condition, CARRYBIT)
#define SET_INTERRUPT(condition)    SET_CONDITION(condition, INTERRUPTBIT)
#define SET_DECIMAL(condition)      SET_CONDITION(condition, DECIMALBIT)
#define SET_BREAK(condition)        SET_CONDITION(condition, BREAKBIT)
#define SET_OVERFLOW(condition)     SET_CONDITION(condition, OVERFLOWBIT)

// relative
#define REL_ADDR(PC, SRC)   ((PC) + ((displacement_t) (SRC)))
#define PAGE(addr)          ((addr) >> 8)
#define GET_SR              SR
#define SET_SR(value)       (SR = value)
#define PULL                M->readbyte(M, STACK_BASE + ++ SP)
#define PUSH(value)         M->writebyte(M, STACK_BASE + SP --, (value))
#define LOAD(addr)          M->readbyte(M, (addr))
#define STORE(addr, value)  M->writebyte(M, (addr), (value))

static void ADC(cnes_cpu_s* cpu)
{
    TEMP = SRC + AC + (CARRY ? 1 : 0);
    SET_ZERO(TEMP & 0xff);
    if (DECIMAL) {
        if (((AC & 0x0f) + (SRC & 0x0f) + (CARRY ? 1 : 0)) > 9) {
            TEMP += 6;
        }
        SET_SIGN(TEMP);
        SET_OVERFLOW(!((AC ^ SRC) & 0x80) && ((AC ^ TEMP) & 0x80));
        if (TEMP > 0x99) {
            TEMP += 96;
        }
        SET_CARRY(TEMP > 0x99);
    } else {
        SET_SIGN(TEMP);
        SET_OVERFLOW(!((AC ^ SRC) & 0x80) && ((AC ^ TEMP) & 0x80));
        SET_CARRY(TEMP > 0xff);
    }
    AC = (byte) TEMP;
}

static void AND(cnes_cpu_s* cpu)
{
    SRC &= AC;
    SET_SIGN(SRC);
    SET_ZERO(SRC);
    AC = SRC;
}

static void ASL(cnes_cpu_s* cpu)
{
    SET_CARRY(SRC & 0x80);
    SRC <<= 1;
    SRC &= 0xff;
    SET_SIGN(SRC);
    SET_ZERO(SRC);
    if (ADDRESSING == M_ACCUMULATOR) {
        AC = SRC;
    } else {
        STORE(ADDR, SRC);
    }
}

static void BCC(cnes_cpu_s* cpu)
{
    if (!CARRY) {
        CYCLES -= cpu->BC ? 2 : 1;
        PC = ADDR;
    }
}

static void BCS(cnes_cpu_s* cpu)
{
    if (CARRY) {
        CYCLES -= cpu->BC ? 2 : 1;
        PC = ADDR;
    }
}

static void BEQ(cnes_cpu_s* cpu)
{
    if (ZERO) {
        CYCLES -= cpu->BC ? 2 : 1;
        PC = ADDR;
    }
}

static void BIT(cnes_cpu_s* cpu)
{
    SET_SIGN(SRC);
    SET_OVERFLOW(SRC & 0x40);
    SET_ZERO(SRC & AC);
}

static void BMI(cnes_cpu_s* cpu)
{
    if (SIGN) {
        CYCLES -= cpu->BC ? 2 : 1;
        PC = ADDR;
    }
}

static void BNE(cnes_cpu_s* cpu)
{
    if (!ZERO) {
        CYCLES -= cpu->BC ? 2 : 1;
        PC = ADDR;
    }
}

static void BPL(cnes_cpu_s* cpu)
{
    if (!SIGN) {
        CYCLES -= cpu->BC ? 2 : 1;
        PC = ADDR;
    }
}

static void BRK(cnes_cpu_s* cpu)
{
    PC += 1;
    cpu->interrupt = INT_BRK;
}

static void BVC(cnes_cpu_s* cpu)
{
    if (!OVERFLOW) {
        CYCLES -= cpu->BC ? 2 : 1;
        PC = ADDR;
    }
}

static void BVS(cnes_cpu_s* cpu)
{
    if (OVERFLOW) {
        CYCLES -= cpu->BC ? 2 : 1;
        PC = ADDR;
    }
}

static void CLC(cnes_cpu_s* cpu)
{
    SET_CARRY(0);
}

static void CLD(cnes_cpu_s* cpu)
{
    SET_DECIMAL(0);
}

static void CLI(cnes_cpu_s* cpu)
{
    SET_INTERRUPT(0);
}

static void CLV(cnes_cpu_s* cpu)
{
    SET_OVERFLOW(0);
}

static void CMP(cnes_cpu_s* cpu)
{
    TEMP = AC - SRC;
    SET_CARRY(TEMP < 0x100);
    SET_SIGN(TEMP);
    SET_ZERO(TEMP & 0xff);
    SRC = (byte) TEMP;
}

static void CPX(cnes_cpu_s* cpu)
{
    TEMP = XR - SRC;
    SET_CARRY(TEMP < 0x100);
    SET_SIGN(TEMP);
    SET_ZERO(TEMP & 0xff);
    SRC = (byte) TEMP;
}

static void CPY(cnes_cpu_s* cpu)
{
    TEMP = YR - SRC;
    SET_CARRY(TEMP < 0x100);
    SET_ZERO(TEMP &= 0xff);
    SET_SIGN(TEMP);
    SRC = (byte) TEMP;
}

static void DEC(cnes_cpu_s* cpu)
{
    SRC = SRC - 1;
    SET_SIGN(SRC);
    SET_ZERO(SRC);
    STORE(ADDR, SRC);
}

static void DEX(cnes_cpu_s* cpu)
{
    SRC = XR - 1;
    SET_SIGN(SRC);
    SET_ZERO(SRC);
    XR = SRC;
}

static void DEY(cnes_cpu_s* cpu)
{
    SRC = YR - 1;
    SET_SIGN(SRC);
    SET_ZERO(SRC);
    YR = SRC;
}

static void EOR(cnes_cpu_s* cpu)
{
    SRC ^= AC;
    SET_SIGN(SRC);
    SET_ZERO(SRC);
    AC = SRC;
}

static void INC(cnes_cpu_s* cpu)
{
    SRC = SRC + 1;
    SET_SIGN(SRC);
    SET_ZERO(SRC);
    STORE(ADDR, SRC);
}

static void INX(cnes_cpu_s* cpu)
{
    SRC = XR + 1;
    SET_SIGN(SRC);
    SET_ZERO(SRC);
    XR = SRC;
}

static void INY(cnes_cpu_s* cpu)
{
    SRC = YR + 1;
    SET_SIGN(SRC);
    SET_ZERO(SRC);
    YR = SRC;
}

static void JMP(cnes_cpu_s* cpu)
{
    PC = ADDR;
}

static void JSR(cnes_cpu_s* cpu)
{
    PC -= 1;
    PUSH((PC >> 8) & 0xff);
    PUSH(PC & 0xff);
    PC = ADDR;
}

static void LDA(cnes_cpu_s* cpu)
{
    SET_SIGN(SRC);
    SET_ZERO(SRC);
    AC = SRC;
}

static void LDX(cnes_cpu_s* cpu)
{
    SET_SIGN(SRC);
    SET_ZERO(SRC);
    XR = SRC;
}

static void LDY(cnes_cpu_s* cpu)
{
    SET_SIGN(SRC);
    SET_ZERO(SRC);
    YR = SRC;
}

static void LSR(cnes_cpu_s* cpu)
{
    SET_CARRY(SRC & 0x01);
    SRC >>= 1;
    SET_SIGN(SRC);
    SET_ZERO(SRC);
    if (ADDRESSING == M_ACCUMULATOR) {
        AC = SRC;
    } else {
        STORE(ADDR, SRC);
    }
}

static void NOP(cnes_cpu_s* cpu)
{
}

static void ORA(cnes_cpu_s* cpu)
{
    SRC |= AC;
    SET_SIGN(SRC);
    SET_ZERO(SRC);
    AC = SRC;
}

static void PHA(cnes_cpu_s* cpu)
{
    SRC = AC;
    PUSH(SRC);
}

static void PHP(cnes_cpu_s* cpu)
{
    SRC = GET_SR;
    PUSH(SRC);
}

static void PLA(cnes_cpu_s* cpu)
{
    SRC = PULL;
    SET_SIGN(SRC);
    SET_ZERO(SRC);
    AC = SRC;
}

static void PLP(cnes_cpu_s* cpu)
{
    SRC = PULL;
    SET_SR(SRC);
}

static void ROL(cnes_cpu_s* cpu)
{
    TEMP = SRC << 1;
    if (CARRY) {
        TEMP |= 0x01;
    }
    SET_CARRY(TEMP > 0xff);
    SRC = (byte) TEMP;
    SET_SIGN(SRC);
    SET_ZERO(SRC);
    if (ADDRESSING == M_ACCUMULATOR) {
        AC = SRC;
    } else {
        STORE(ADDR, SRC);
    }
}

static void ROR(cnes_cpu_s* cpu)
{
    TEMP = SRC;
    if (CARRY) {
        TEMP |= 0x100;
    }
    SET_CARRY(TEMP & 0x01);
    TEMP >>= 1;
    SRC = (byte) TEMP;
    SET_SIGN(SRC);
    SET_ZERO(SRC);
    if (ADDRESSING == M_ACCUMULATOR) {
        AC = SRC;
    } else {
        STORE(ADDR, SRC);
    }
}

static void RTI(cnes_cpu_s* cpu)
{
    SRC = PULL;
    SET_SR(SRC);
    PC = PULL;
    PC |= (PULL << 8);
}

static void RTS(cnes_cpu_s* cpu)
{
    PC = PULL;
    PC |= (PULL << 8);
    PC += 1;
}

static void SBC(cnes_cpu_s* cpu)
{
    TEMP = AC - SRC - (CARRY ? 0 : 1);
    SET_SIGN(TEMP);
    SET_ZERO(TEMP & 0xff);
    SET_OVERFLOW(((AC ^ TEMP) & 0x80) && ((AC ^ SRC) & 0x80));
    if (DECIMAL) {
        if (((AC & 0x0f) - (CARRY ? 0 : 1)) < (SRC & 0x0f)) {
            TEMP -= 6;
        }
        if (TEMP > 0x99) {
            TEMP -= 0x60;
        }
    }
    SET_CARRY(TEMP < 0x100);
    AC = (byte) TEMP;
}

static void SEC(cnes_cpu_s* cpu)
{
    SET_CARRY(1);
}

static void SED(cnes_cpu_s* cpu)
{
    SET_DECIMAL(1);
}

static void SEI(cnes_cpu_s* cpu)
{
    SET_INTERRUPT(1);
}

static void STA(cnes_cpu_s* cpu)
{
    SRC = AC;
    STORE(ADDR, SRC);
}

static void STX(cnes_cpu_s* cpu)
{
    SRC = XR;
    STORE(ADDR, SRC);
}

static void STY(cnes_cpu_s* cpu)
{
    SRC = YR;
    STORE(ADDR, SRC);
}

static void TAX(cnes_cpu_s* cpu)
{
    SRC = AC;
    SET_SIGN(SRC);
    SET_ZERO(SRC);
    XR = SRC;
}

static void TAY(cnes_cpu_s* cpu)
{
    SRC = AC;
    SET_SIGN(SRC);
    SET_ZERO(SRC);
    YR = SRC;
}

static void TSX(cnes_cpu_s* cpu)
{
    SRC = SP;
    SET_SIGN(SRC);
    SET_ZERO(SRC);
    XR = SRC;
}

static void TXA(cnes_cpu_s* cpu)
{
    SRC = XR;
    SET_SIGN(SRC);
    SET_ZERO(SRC);
    AC = SRC;
}

static void TXS(cnes_cpu_s* cpu)
{
    SRC = XR;
    SP = SRC;
}

static void TYA(cnes_cpu_s* cpu)
{
    SRC = YR;
    SET_SIGN(SRC);
    SET_ZERO(SRC);
    AC = SRC;
}

void cnes_cpu_init(cnes_cpu_s* cpu, cnes_memory_s* memory)
{
    CLEAR(cpu);
    SR = 0x34;
    SP = 0xfd;
    cpu->interrupt = INT_RESET;
    M = memory;
}

void cnes_cpu_reset(cnes_cpu_s* cpu)
{
    SP -= 3;
    SET_INTERRUPT(1);
}

void cnes_cpu_dump(const cnes_cpu_s* cpu)
{
    printf("AC: %02x\tXR: %02x\tYR: %02x\n", AC, XR, YR);
    printf("PC: %04x\tSP: %04x\n", PC, SP);
    printf("SR: %c%c%c%c%c%c%c%c\n", 
        SIGN ?      'S' : 's',
        OVERFLOW ?  'V' : 'v',
        test_bit(SR, 5) ? 'R' : 'r',
        BREAK ?     'B' : 'b',
        DECIMAL ?   'D' : 'd',
        INTERRUPT ? 'I' : 'i',
        ZERO ?      'Z' : 'z',
        CARRY ?     'C' : 'c');
}

void cnes_cpu_dumpb(FILE* fout, const cnes_cpu_s* cpu)
{
    fwrite(cpu, sizeof (cnes_cpu_s), 1, fout);
}

void cnes_cpu_loadb(cnes_cpu_s* cpu, FILE* fin)
{
    cnes_memory_s* memory = cpu->memory;
    fread(cpu, sizeof (cnes_cpu_s), 1, fin);
    cpu->memory = memory;
}

static word interrupt_handlers[4] = {
    NMI_HANDLER, RESET_HANDLER, IRQ_HANDLER, IRQ_HANDLER
};

static void handle_interrupt(cnes_cpu_s* cpu)
{
    if (cpu->interrupt == INT_NONE) {
        return;
    }
    assert(cpu->interrupt == INT_NMI || cpu->interrupt == INT_RESET || 
        cpu->interrupt == INT_IRQ || cpu->interrupt == INT_BRK);
    PUSH((PC >> 8) & 0xff);
    PUSH(PC & 0xff);
    if (cpu->interrupt == INT_BRK) {
        SET_BREAK(1);
    }
    PUSH(SR);
    SET_INTERRUPT(1);
    PC = M->readword(M, interrupt_handlers[cpu->interrupt]);
    if (cpu->interrupt != INT_RESET) {
        CYCLES -= 7;
    }
    cpu->interrupt = INT_NONE;
}

typedef void (*implement_instruction_func)(cnes_cpu_s* cpu);

const implement_instruction_func funcs[CNES_INSTRUCTION_NUMBER] =  {
    BRK,	ORA,	NULL,	NULL,	NULL,
    ORA,	ASL,	NULL,	PHP,	ORA,
    ASL,	NULL,	NULL,	ORA,	ASL,
    NULL,	BPL,	ORA,	NULL,	NULL,
    NULL,	ORA,	ASL,	NULL,	CLC,
    ORA,	NULL,	NULL,	NULL,	ORA,
    ASL,	NULL,	JSR,	AND,	NULL,
    NULL,	BIT,	AND,	ROL,	NULL,
    PLP,	AND,	ROL,	NULL,	BIT,
    AND,	ROL,	NULL,	BMI,	AND,
    NULL,	NULL,	NULL,	AND,	ROL,
    NULL,	SEC,	AND,	NULL,	NULL,
    NULL,	AND,	ROL,	NULL,	RTI,
    EOR,	NULL,	NULL,	NULL,	EOR,
    LSR,	NULL,	PHA,	EOR,	LSR,
    NULL,	JMP,	EOR,	LSR,	NULL,
    BVC,	EOR,	NULL,	NULL,	NULL,
    EOR,	LSR,	NULL,	CLI,	EOR,
    NULL,	NULL,	NULL,	EOR,	LSR,
    NULL,	RTS,	ADC,	NULL,	NULL,
    NULL,	ADC,	ROR,	NULL,	PLA,
    ADC,	ROR,	NULL,	JMP,	ADC,
    ROR,	NULL,	BVS,	ADC,	NULL,
    NULL,	NULL,	ADC,	ROR,	NULL,
    SEI,	ADC,	NULL,	NULL,	NULL,
    ADC,	ROR,	NULL,	NULL,	STA,
    NULL,	NULL,	STY,	STA,	STX,
    NULL,	DEY,	NULL,	TXA,	NULL,
    STY,	STA,	STX,	NULL,	BCC,
    STA,	NULL,	NULL,	STY,	STA,
    STX,	NULL,	TYA,	STA,	TXS,
    NULL,	NULL,	STA,	NULL,	NULL,
    LDY,	LDA,	LDX,	NULL,	LDY,
    LDA,	LDX,	NULL,	TAY,	LDA,
    TAX,	NULL,	LDY,	LDA,	LDX,
    NULL,	BCS,	LDA,	NULL,	NULL,
    LDY,	LDA,	LDX,	NULL,	CLV,
    LDA,	TSX,	NULL,	LDY,	LDA,
    LDX,	NULL,	CPY,	CMP,	NULL,
    NULL,	CPY,	CMP,	DEC,	NULL,
    INY,	CMP,	DEX,	NULL,	CPY,
    CMP,	DEC,	NULL,	BNE,	CMP,
    NULL,	NULL,	NULL,	CMP,	DEC,
    NULL,	CLD,	CMP,	NULL,	NULL,
    NULL,	CMP,	DEC,	NULL,	CPX,
    SBC,	NULL,	NULL,	CPX,	SBC,
    INC,	NULL,	INX,	SBC,	NOP,
    NULL,	CPX,	SBC,	INC,	NULL,
    BEQ,	SBC,	NULL,	NULL,	NULL,
    SBC,	INC,	NULL,	SED,	SBC,
    NULL,	NULL,	NULL,	SBC,	INC,
    NULL
};

static void cpu_fetch(cnes_cpu_s* cpu)
{
    OPCODE = M->readbyte(M, PC);
    if (CNES_BAD_OPCODE(OPCODE)) {
        printf("PC: %04x %02x\n", PC, OPCODE);
        assert(!CNES_BAD_OPCODE(OPCODE));
    }
    PC += 1;
    const cnes_instruction_s* instruction = &instructions[OPCODE];
    switch (instruction->bytes) {
        case 1:
            OPERAND = 0;
            break;
        case 2:
            OPERAND = LOAD(PC);
            PC += 1;
            break;
        case 3:
            OPERAND = M->readword(M, PC);
            PC += 2;
            break;
        default:
            assert(FALSE);
    }
}

static void cpu_address(cnes_cpu_s* cpu)
{
    cpu->BC = FALSE;
    switch (instructions[OPCODE].addressing) {
        case M_ACCUMULATOR:
            SRC = AC;
            break;
        case M_IMMEDIATE:
            SRC = OPERAND;
            break;
        case M_IMPLIED:
            break;
        case M_ZEROPAGE:
        case M_ABSOLUTE:
            ADDR = OPERAND;
            // JMP JSR STA STX STY
            if (OPCODE != 0x4c && OPCODE != 0x20 && 
                OPCODE != 0x8d && OPCODE != 0x8e && OPCODE != 0x8c) {
                SRC = LOAD(ADDR);
            }
            break;
        case M_XINDEXED_ZEROPAGE:
            ADDR = (byte) (OPERAND + XR);
            SRC = LOAD(ADDR);
            break;
        case M_YINDEXED_ZEROPAGE:
            ADDR = (byte) (OPERAND + YR);
            SRC = LOAD(ADDR);
            break;
        case M_XINDEXED:
            ADDR = OPERAND + XR;
            if (PAGE(OPERAND) != PAGE(ADDR) && 
                instructions[OPCODE].cycles == 4) {
                cpu->BC = TRUE;
                CYCLES -= 1;
            }
            SRC = LOAD(ADDR);
            break;
        case M_YINDEXED:
            ADDR = OPERAND + YR;
            if (PAGE(OPERAND) != PAGE(ADDR) && 
                instructions[OPCODE].cycles == 4) {
                cpu->BC = TRUE;
                CYCLES -= 1;
            }
            SRC = LOAD(ADDR);
            break;
        case M_INDIRECT:
            ADDR = OPERAND;
            if ((ADDR & 0xff) == 0xff) {
                ADDR = LOAD(ADDR & 0xff00) << 8 | LOAD(ADDR);
            } else {
                ADDR = M->readword(M, ADDR);
            }
            break;
        case M_INDEXED_INDIRECT:
            ADDR = M->readword(M, (byte) (OPERAND + XR));
            SRC = LOAD(ADDR);
            break;
        case M_INDIRET_INDEXED: {
                word temp = M->readword(M, OPERAND);
                ADDR = temp + YR;
                if (PAGE(ADDR) != PAGE(temp) && 
                    instructions[OPCODE].cycles == 5) {
                    cpu->BC = TRUE;
                    CYCLES -= 1;
                }
                SRC = LOAD(ADDR);
            }
            break;
        case M_RELATIVE:
            SRC = OPERAND;
            ADDR = REL_ADDR(PC, SRC);
            if (PAGE(ADDR) != PAGE(PC)) {
                cpu->BC = TRUE;
            }
            break;
        default:
            assert(FALSE);
    }
}

uint cnes_cpu_step(cnes_cpu_s* cpu, BOOL dump)
{
    handle_interrupt(cpu);
    word temp = PC;
    cpu_fetch(cpu);
    if (dump) {
        cnes_dump_instruction(temp, OPCODE, OPERAND);
    }
    cpu_address(cpu);
    assert(NOTNULL(funcs[OPCODE]));
    funcs[OPCODE](cpu);
    SET_BREAK(1);
    set_bit(&SR, 5);
    return instructions[OPCODE].cycles;
}

void cnes_cpu_cycle(cnes_cpu_s* cpu, sint cycles, BOOL dump)
{
    CYCLES += cycles;
    while (CYCLES > 0) {
        CYCLES -= cnes_cpu_step(cpu, dump);
    }
}

void cnes_cpu_signal(cnes_cpu_s* cpu, cnes_interrupt_t interrupt)
{
    if (interrupt == INT_IRQ) {
        if (!INTERRUPT) {
            cpu->interrupt = interrupt;
        } 
    } else {
        cpu->interrupt = interrupt;
    }
}