// types.h
// Author: hyan23
// Date: 2018.04.21

#ifndef __TYPES_H__
#define __TYPES_H__

#include <stdint.h>

#define     BOOL        int
#define     TRUE        1
#define     FALSE       0

typedef uint8_t         byte;
typedef uint16_t        word;
typedef byte            bit;

typedef unsigned int    uint;
typedef int             sint;

#define RAW_PTR(ptr)    ((byte*)(ptr))

#define BUFFER_SMALL    0x10
#define BUFFER_MIDDLE   0x100
#define BUFFER_LARGE    0x1000
#define FILENAME        BUFFER_MIDDLE

#endif /* __TYPES_H__ */