// ini.h
// Author: hyan23
// Date: 2018.05.27

#ifndef __INI_H__
#define __INI_H__

// simple ini file parser
// supported features:
// 1. ; # comment sign
// 2. blank lines
// 3. allow duplicate section names(merge duplicate sections into one)
// 4. case sensitivity(not case sensitive)
// 5. member selection operator
// unsupported features:
// 1. multi-line
// 2. section nesting
// 3. escape characters
// reserved words:
// semi-colon; equals sign= number sign# square brackets[]
// parser characteristics
// 1. ignores leading and trailling whitespaces around property names
// and values
// 2. the second occurrence of a property whthin 
// one section will overwrite the value of the previous one
// 3. value types: signed integer, floating point, string, 
// boolean(true, false)
// attributes:
// 1. identifier length: 63 characters
// 2. value length: 63 characters
// 3. comment length(including multi-line): 254 characters

#include "header.h"
#include "array.h"
#include "pair.h"
#include "section.h"

typedef struct INI_FILE {
    char file[FILENAME];
    FILE* fin;
    array_s sections;
    error_t result;
}
ini_file_s;

#define INI_SECTION_NUMBER      5
#define INI_SECTION_INC         2

#define COMMENT_SIGN(ch)    ((ch) == ';' || (ch) == '#')
#define SECTION_SIGN(ch)    ((ch) == '[')
#define MEMBER_SEL_OPERATORS    ".>"
#define MEMBER_SEL_OPERATOR(ch) FOUND(find(MEMBER_SEL_OPERATORS, (ch)))

extern error_t ini_open(ini_file_s* ini, const char* filename);
extern void ini_close(ini_file_s* ini);
extern void ini_dump(FILE* fout, const ini_file_s* ini);
// sname: section name
extern ini_pair_s* ini_pair(ini_file_s* ini, 
    const char* sname, const char* property);
extern ini_pair_s* ini_append(ini_file_s* ini, 
    const char* sname, const char* property, const char* value);
// sname == NULL AND property == NULL removes all sections along with their pairs, 
// property == NULL removes a section specified by its name
extern void ini_remove(ini_file_s* ini, const char* sname, const char* property);
extern error_t ini_string(ini_file_s* ini, 
    const char* sname, const char* property, char* string);
extern error_t ini_numeric(ini_file_s* ini, 
    const char* sname, const char* property, double* value);
extern error_t ini_boolean(ini_file_s* ini, 
    const char* sname, const char* property, BOOL* value);
    
// last member selection position
extern const char* ini_member(const char* expr);
// parse member selection expression
extern error_t member_selction(ini_file_s* ini, 
    char (*sname)[INI_SECTION_NAME_LEN + 1], 
    char (*property)[INI_PAIR_NAME_LEN + 1], 
    const char* member);

#define INI_PREPARE \
    double __ini_temp;  \
    (void) __ini_temp;  \
    error_t __ini_result;   \
    (void) __ini_result;    \
    char __ini_sname[INI_SECTION_NAME_LEN + 1]; \
    (void) __ini_sname; \
    char __ini_property[INI_PAIR_NAME_LEN + 1]; \
    (void) __ini_property;
    
#define MEMBER_SELECTION(ini, expr) \
    __ini_result = member_selction((ini),   \
    &__ini_sname, &__ini_property, ini_member(#expr))
    
// uses member selection operator
#define INI_PAIR(ini, expr) \
    (MEMBER_SELECTION((ini), expr), \
    SUCCEED(__ini_result) ? \
    ini_pair((ini), __ini_sname, __ini_property) : NULL)

#define INI_APPEND(ini, expr, value)    \
    (MEMBER_SELECTION((ini), expr), \
    SUCCEED(__ini_result) ? \
    ini_append((ini), __ini_sname, __ini_property, (value)) : NULL)

#define INI_REMOVE(ini, expr)   \
    MEMBER_SELECTION((ini), expr);  \
    if (SUCCEED(__ini_result) { \
        ini_remove((ini), __ini_sname, __ini_property); \
    }

#define INI_STRING(ini, value)  \
    (__ini_result = (MEMBER_SELECTION((ini), value),    \
    __ini_result = SUCCEED(__ini_result) ?  \
    ini_string((ini), __ini_sname, __ini_property, (value)) : __ini_result, \
    __ini_result))

#define INI_NUMERIC(ini, value) \
    (__ini_result = (MEMBER_SELECTION((ini), value),    \
    __ini_result = SUCCEED(__ini_result) ?  \
    ini_numeric((ini), __ini_sname, __ini_property, &__ini_temp) :  \
    __ini_result,   \
    *(value) = SUCCEED(__ini_result) ? __ini_temp : *(value),   \
    __ini_result))
    
#define INI_BOOLEAN(ini, value) \
    (__ini_result = (MEMBER_SELECTION((ini), value),    \
    __ini_result = SUCCEED(__ini_result) ?  \
    ini_boolean((ini), __ini_sname, __ini_property, (value)) : __ini_result,    \
    __ini_result))

#endif /* __INI_H__ */