// section.c
// Author: hyan23
// Date: 2018.05.28

#include "header.h"
#include "charstar.h"
#include "array.h"
#include "pair.h"
#include "section.h"

error_t ini_section_init(ini_section_s* section, const char* sname)
{
    CLEAR(section);
    if (NUL(sname)) {
        sname = INI_SECTION_NAME;
    }
    strncpy(section->sname, sname, INI_SECTION_NAME_LEN + 1);
    return array_create(&section->pairs, ini_pair_s, 
        INI_PAIR_NUMBER, INI_PAIR_INC);
}

void ini_section_destory(ini_section_s* section)
{
    array_destroy(&section->pairs, NULL);
}

void ini_section_dump(FILE* fout, const ini_section_s* section)
{
    fprintf(fout, "[%s]\n", section->sname);
    array_dump(fout, &section->pairs, (array_dump_func) ini_pair_dump);
}

static sint find_pair(ini_pair_s* pair, const char* property)
{
    return STRIEQU(pair->name, property) ? ARRAY_CMP_EQUAL : -1;
}

ini_pair_s* 
ini_section_append(ini_section_s* section, const ini_pair_s* pair)
{
    ini_pair_s* temp = array_find(&section->pairs, 
        pair->name, (array_cmp_func) find_pair);
    if (NOTNULL(temp)) {
        strncpy(temp->value, pair->value, INI_PAIR_VAL_LEN + 1);
        return temp;
    } else {
        return array_append(&section->pairs, pair);
    }
}

void ini_section_remove(ini_section_s* section, const char* property)
{
    array_remove(&section->pairs, property, (array_cmp_func) find_pair);
}

ini_pair_s* ini_section_pair(ini_section_s* section, const char* property)
{
    return (ini_pair_s*) array_find(&section->pairs, property, 
        (array_cmp_func) find_pair);
}