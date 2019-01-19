// list.h
// Author: hyan23
// Date: 2017.08.08

#ifndef __LIST_H__
#define __LIST_H__

#include "header.h"

typedef void (*list_dump_func)(FILE* fout, const void* list);
#define LIST_CMP_EQUAL      0
typedef sint (*list_cmp_func)(const void* self, const void* object);
typedef void (*list_destroy_func)(void* list);

#endif /* __LIST_H__ */