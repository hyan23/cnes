// array.c
// Author: hyan23
// Date: 2017.08.02

#include "header.h"
#include "array.h"

#define ARRAY_ELEMENT(list, index)  \
    ((list)->array + (index) * (list)->_size)

error_t 
_array_create(array_s* list, 
uint _size, uint _capacity, uint _inc) 
{
    CLEAR(list);
    list->array = malloc(_size * _capacity);
    if (NUL(list->array)) {
        return MEMORY_OVERFLOW;
    }
    list->count = 0;
    list->capacity = _capacity;
    list->_size = _size;
    list->_inc = _inc;
    return SUCCESS;
}

void _array_destroy(array_s* list, array_destroy_func destroy)
{
    if (NOTNULL(destroy)) {
        for (uint i = 0; i < list->count; i ++) {
            destroy(ARRAY_ELEMENT(list, i));
        }
    }
    FREE(list->array);
}

uint _array_capacity(const array_s* list)
{
    return list->capacity;
}

uint _array_count(const array_s* list)
{
    return list->count;
}

void _array_dump(FILE* fout, const array_s* list, array_dump_func dump)
{
    for (uint i = 0; i < list->count; i ++) {
        dump(fout, ARRAY_ELEMENT(list, i));
    }
}

void* _array_append(array_s* list, const void* element)
{
    if (list->count == list->capacity) {
        void* ptr = realloc(list->array, 
            (list->capacity + list->_inc) * list->_size);
        if (NUL(ptr)) {
            return NULL;
        }
        list->array = ptr;
        list->capacity += list->_inc;
    }
    void* dest = ARRAY_ELEMENT(list, list->count);
    memcpy(dest, element, list->_size);
    list->count ++;
    return dest;
}

void* _array_get(array_s* list, uint index)
{
    if (index >= list->count) {
        return NULL;
    } else {
        return ARRAY_ELEMENT(list, index);
    }
}

void* _array_find(array_s* list, const void* object, 
    array_cmp_func cmp) {
    for (uint i = 0; i < list->count; i ++) {
        void* self = ARRAY_ELEMENT(list, i);
        if (cmp(self, object) == ARRAY_CMP_EQUAL) {
            return self;
        }
    }
    return NULL;
}

void _array_erase(array_s* list, uint index)
{
    if (index < list->count) {
        for (int i = index; i < list->count - 1; i ++) {
            memcpy(ARRAY_ELEMENT(list, i), ARRAY_ELEMENT(list, i + 1), 
                list->_size);
        }
        list->count --;
    }
}

void 
_array_remove(array_s* list, const void* object, 
array_cmp_func cmp)
{
    for (int i = 0; i < list->count; i ++) {
        if (cmp(ARRAY_ELEMENT(list, i), object) == 
            ARRAY_CMP_EQUAL) {
            _array_erase(list, i);
            break;
        }
    }
}