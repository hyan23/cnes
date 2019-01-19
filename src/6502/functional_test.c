// functional_test.c
// Author: hyan23
// Date: 2017.08.23

#include <time.h>
#include "header.h"
#include "common.h"
#include "memory.h"
#include "cpu.h"

static sint 
runtest(cnes_memory_s* memory, word* program_counter, 
word outlet, unsigned long* n, time_t sec) 
{
    cnes_cpu_s cpu;
    cnes_cpu_init(&cpu, memory);
    cpu.interrupt = INT_NONE;
    cpu.program_counter = *program_counter;
    
    struct timespec start;
    CLEAR(&start);
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &start);
    time_t now = time(NULL);
    
    while (cpu.program_counter != outlet) {
        if (time(NULL) - now > sec) {
            *program_counter = cpu.program_counter;
            cnes_cpu_dump(&cpu);
            return -1;
        }
        cnes_cpu_step(&cpu, FALSE);
        (*n) ++;
    }
    
    struct timespec stop;
    CLEAR(&stop);
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &stop);
    return INTERVAL(start, stop);
}

int main(void)
{
    byte* rom = MALLOC(byte, 0x10000);
    assert(NOTNULL(rom));
    FILE* fin = fopen("6502_functional_test.bin", "rb");
    assert(NOTNULL(fin));
    fseek(fin, 0, SEEK_END);
    uint size = ftell(fin);
    rewind(fin);
    assert(fread(rom, size, 1, fin) == 1);
    
    cnes_memory_s memory;
    CLEAR(&memory);
    memory.layout = rom;
    memory.readbyte = cnes_memory_readbyte;
    memory.readword = cnes_memory_readword;
    memory.writebyte = cnes_memory_writebyte;
    memory.writeword = cnes_memory_writeword;
    
    word program_counter = 0x400;
    unsigned long n = 0;
    sint usec = runtest(&memory, &program_counter, 0x3399, &n, 4);
    if (usec != -1) {
        printf("success, %lu instructions have been executed, "
            "total %d useconds, %.2lf instructions per usecond.\n", 
            n, usec, (double) n / usec);
    } else {
        printf("stuck at %04x\n", program_counter);
    }
    
    FREE(rom);
    fclose(fin);
}