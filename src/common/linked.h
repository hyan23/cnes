// linked.h
// Author: hyan23
// Date: 2017.08.06

#ifndef __LINKED_H__
#define __LINKED_H__

#include "header.h"
#include "list.h"

// member
#define sizeofm(type, field)    sizeof(((type*) 0)->field)

typedef uint linked_field_t;
#define LINKED_FIELD(type, field)   \
    ((linked_field_t) offsetof (type, field))

#define linked_dump_func        list_dump_func
#define LINKED_CMP_EQUAL        LIST_CMP_EQUAL
#define linked_cmp_func         list_cmp_func
#define linked_destroy_func     list_destroy_func

extern void linked_create_(void** list);
extern void* linked_append_(void** list, const void* data, 
    uint _data, uint _datasize, uint _next, uint _size);
extern void linked_dump_(FILE* fout, const void** list, 
    linked_dump_func dump, uint _next);
extern void* linked_find_(const void** list, const void* object, 
    linked_cmp_func cmp, uint _field, uint _next);
void* linked_remove_(void** list, void** prev, const void* object, 
    linked_cmp_func cmp, uint _field, uint _next);
extern void linked_destroy_(void** list, 
    linked_destroy_func destroy, uint _next);

#define _linked_create(linked, list)    \
    linked_create_((void**) (&(list)));

#define _linked_append(linked, list, _data) \
    ((linked*) linked_append_((void**) (&(list)), (&(_data)),   \
        offsetof(linked, data), sizeofm(linked, data),  \
        offsetof(linked, next), sizeof (linked)))

#define _linked_dump(fout, linked, list, destroy) \
    linked_dump_((fout), (const void**) (&(list)), \
        (destroy), offsetof(linked, next))

#define _linked_find(linked, list, object, cmp, field) \
    ((linked*) linked_find_((const void**) (&list), \
        (const void*) (&(object)), \
        (cmp), offsetof(linked, field), offsetof(linked, next)))

// uses linked_field_t
#define __linked_find(linked, list, object, cmp, field) \
    ((linked*) linked_find_((const void**) (&list), \
        (const void*) (&(object)), \
        (cmp), field, offsetof(linked, next)))

#define _linked_remove(linked, list, object, cmp, field) \
    ((linked*) linked_remove_((void**) (&list), \
        (void**) (&list), (const void*) (&(object)), \
        (cmp), offsetof(linked, field), offsetof(linked, next)))

// uses linked_field_t
#define __linked_remove(linked, list, object, cmp, field) \
    ((linked*) linked_remove_((void**) (&list), \
        (void**) (&list), (const void*) (&(object)), \
        (cmp), field, offsetof(linked, next)))

#define _linked_destroy(linked, list, destroy) \
    linked_destroy_((void**) (&(list)), \
        (destroy), offsetof (linked, next))

#define LINKED_DECL_LIST(linked, postfix) \
    extern void linked_create_##postfix(linked** list); \
    extern linked* linked_append_##postfix(linked** list, void* data); \
    extern void linked_dump_##postfix(FILE* fout, linked** list, \
        linked_dump_func dump); \
    extern linked* linked_find_##postfix(linked** list, const void* object, \
        linked_cmp_func cmp, linked_field_t field); \
    extern linked* linked_remove_##postfix(linked** list, const void* object, \
        linked_cmp_func cmp, linked_field_t field); \
    extern void linked_destroy_##postfix(linked** list, \
        linked_destroy_func destroy);

#define LINKED_IMPL_LIST(linked, postfix) \
    void linked_create_##postfix(linked** list) \
    { _linked_create(linked, *list); } \
    linked* linked_append_##postfix(linked** list, void* data) \
    { return _linked_append(linked, *list, *((byte**) data)); } \
    void linked_dump_##postfix(FILE* fout, linked** list, linked_dump_func dump) \
    { _linked_dump(fout, linked, *list, dump); } \
    linked* linked_find_##postfix(linked** list, const void* object, \
        linked_cmp_func cmp, linked_field_t field) \
    { return __linked_find(linked, *list, *((byte*) object), cmp, field); } \
    linked* linked_remove_##postfix(linked** list, const void* object, \
        linked_cmp_func cmp, linked_field_t field) \
    { return __linked_remove(linked, *list, *((byte*) object), cmp, field); } \
    void linked_destroy_##postfix(linked** list, linked_destroy_func destroy) \
    { _linked_destroy(linked, *list, destroy); }

#endif /* __LINKED_H__ */