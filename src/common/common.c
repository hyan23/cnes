// common.c
// Author: hyan23
// Date: 2017.08.02

#include <time.h>
#include "header.h"
#include "common.h"

char* datetime(char* buf, uint size, const char* format)
{
    time_t now;
    time(&now);
    struct tm* local = localtime(&now);
    strftime(buf, size, 
        NOTNULL(format) ? format : "%m/%d/%Y %H:%M:%S", 
        local);
    return buf;
}

static const char HEX[0x10] = { 
    '0', '1', '2', '3', '4', '5', '6', '7', 
    '8', '9','a', 'b', 'c', 'd', 'e', 'f' };

void putbyte(byte value)
{
    putchar(HEX[value >> 4]);
    putchar(HEX[value & 0xf]);
}

void dump(uint offset, uint row, const void* src, uint size)
{
    if (row == 0) {
        return;
    }
    uint i = 0;
    while (i < size) {
        if (i % row == 0) {
            printf("%s0x%04x ", i != 0 ? "\n" : "", offset + i);
        } else if (i % (row / 2) == 0) {
            putchar(' ');
        }
        putbyte(((const byte*) src)[i]);
        putchar(' ');
        i ++;
    }
    putchar('\n');
}