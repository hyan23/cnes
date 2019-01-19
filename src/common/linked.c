// linked.c
// Author: hyan23
// Date: 2017.08.02

#include "linked.h"

#define L   (*((byte**) list))

void linked_create_(void** list)
{
    L = NULL;
}

void* 
linked_append_(void** list, const void* data, 
uint _data, uint _datasize, uint _next, uint _size) 
{
    if (NUL(L)) {
        L = (byte*) malloc(_size);
        memset(L, 0, _size);
        *((byte**) (L + _next)) = NULL;
        memcpy(L + _data, data, _datasize);
        return (void*) L;
    }
    return linked_append_((void**) (L + _next), 
        data, _data, _datasize, 
        _next, _size);
}

void 
linked_dump_(FILE* fout, const void** list, 
linked_dump_func dump, uint _next) 
{
    if (NOTNULL(L)) {
        if (NOTNULL(dump)) {
            dump(fout, L);
        }
        linked_dump_(fout, (const void**) (L + _next), dump, _next);
    }
}

void* 
linked_find_(const void** list, const void* object, 
linked_cmp_func cmp, uint _field, uint _next) 
{
    if (NOTNULL(L)) {
        if (cmp((const void*) (L + _field), object) == LINKED_CMP_EQUAL) {
            return (void*) L;
        }
        return linked_find_((const void**) (L + _next), object, cmp, 
            _field, _next);
    }
    return NULL;
}

void*  
linked_remove_(void** list, void** prev, const void* object, 
linked_cmp_func cmp, uint _field, uint _next) 
{
    if (NOTNULL(L)) {
        if (cmp((const void*) (L + _field), object) == LINKED_CMP_EQUAL) {
            byte* tmp = L;
            byte* next = *((byte**) (L + _next));
            FREE(L);
            if (list == prev) {
                L = next;
            } else {
                *((byte**) (*((byte**) prev) + _next)) = next;
            }
            return (void*) tmp;
        }
        return linked_remove_((void**) (L + _next), list, object, cmp, 
            _field, _next);
    }
    return NULL;
}

void 
linked_destroy_(void** list, 
linked_destroy_func destroy, uint _next) 
{
    if (NOTNULL(L)) {
        linked_destroy_((void**) (L + _next), destroy, _next);
        if (NOTNULL(destroy)) {
            destroy(L);
        }
        FREE(L);
        L = NULL;
    }
}