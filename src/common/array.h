// array.h
// Author: hyan23
// Date: 2017.08.08

#ifndef __ARRAY_H__
#define __ARRAY_H__

#include "header.h"
#include "list.h"

// TODO: _array_sort
 
typedef struct ARRAY {
    void*       array;
    uint        count;
    uint        capacity;
    uint        _size;
    uint        _inc;
}
array_s;

#define array_dump_func         list_dump_func
#define ARRAY_CMP_EQUAL         LIST_CMP_EQUAL
#define array_cmp_func          list_cmp_func
#define array_destroy_func      list_destroy_func

extern error_t _array_create(array_s* list, 
    uint _size, uint _capacity, uint _inc);
extern void _array_destroy(array_s* list, array_destroy_func destroy);
extern uint _array_capacity(const array_s* list);
extern uint _array_count(const array_s* list);
extern void _array_dump(FILE* fout, const array_s* list, array_dump_func dump);
extern void* _array_append(array_s* list, const void* element);
extern void* _array_get(array_s* list, uint index);
extern void* _array_find(array_s* list, const void* object, 
    array_cmp_func cmp);
extern void _array_erase(array_s* list, uint index);
extern void _array_remove(array_s* list, const void* object, 
    array_cmp_func cmp);

#define array_create(list, type, capacity, inc) \
    _array_create((list), sizeof (type), (capacity), (inc))
#define array_destroy       _array_destroy
#define array_capacity      _array_capacity
#define array_count         _array_count
#define array_dump          _array_dump
#define array_append        _array_append
#define array_get           _array_get
#define array_find          _array_find
#define array_erase         _array_erase
#define array_remove        _array_remove

#endif /* __ARRAY_H__ */