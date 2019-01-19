// section.h
// Author: hyan23
// Date: 2018.05.28

#ifndef __SECTION_H__
#define __SECTION_H__

#include "header.h"
#include "array.h"
#include "pair.h"

#define INI_SECTION_NAME_LEN        (4 * BUFFER_SMALL - 1)
#define INI_SECTION_NAME            "section"

#define INI_PAIR_NUMBER             10
#define INI_PAIR_INC                4

typedef struct INI_SECTION {
    // section name
    char sname[INI_SECTION_NAME_LEN + 1];
    array_s pairs;
}
ini_section_s;

extern error_t ini_section_init(ini_section_s* section, const char* sname);
extern void ini_section_destory(ini_section_s* section);
extern void ini_section_dump(FILE* fout, const ini_section_s* section);
extern ini_pair_s* ini_section_append(ini_section_s* section, 
    const ini_pair_s* pair);
extern void ini_section_remove(ini_section_s* section, const char* property);
extern ini_pair_s* ini_section_pair(ini_section_s* section, 
    const char* property);

#endif /* __SECTION_H__ */