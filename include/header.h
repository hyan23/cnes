// header.h
// Author: hyan23
// Date: 2017.08.01

#ifndef __HEADER_H__
#define __HEADER_H__

#include <stdio.h>
#include <assert.h>
#include <stdlib.h> /* malloc free */
#include <string.h> /* memset */

#include "types.h"
#include "error.h"

// lower bound, upper bound
#define BETWEEN(value, lb, ub)  ((value) >= (lb) && (value) <= (ub))
#define WIDTH(value, width)     (((value) & (~0 << (width))) == 0)

#define set_bit(ptr, bit)       (*(ptr) |= (0x01 << (bit)))
#define reset_bit(ptr, bit)     (*(ptr) &= (~(0x01 << (bit))))
#define test_bit(value, bit)    ((value) & (0x01 << (bit)))

#define MALLOC(type, num)       ((type*) malloc((num) * sizeof (type)))
#define FREE(ptr)               (ptr = NOTNULL(ptr) ? (free(ptr), NULL) : NULL)
#define CLEAR(ptr)              memset((ptr), 0, sizeof (*(ptr)))

#endif /* __HEADER_H__ */