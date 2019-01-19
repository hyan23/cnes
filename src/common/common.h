// common.h
// Author: hyan23
// Date: 2017.08.02

#ifndef __COMMON_H__
#define __COMMON_H__

#include "header.h"

#define INTERVAL(start, stop)                       \
    (((stop).tv_sec - (start).tv_sec) * 1000000 +   \
    ((stop).tv_nsec - (start).tv_nsec) / 1000)
    
extern char* datetime(char* buf, uint size, const char* format);
extern void dump(uint offset, uint row, 
    const void* src, uint size);

#include "array.h"
#include "linked.h"
#include "charstar.h"

#endif /* __COMMON_H__ */