// charstar.h
// Author: hyan23
// Date: 2017.08.05

#ifndef __CHARSTAR_H__
#define __CHARSTAR_H__

#include <string.h>
#include "header.h"
#include "charstar.h"

#define WHITE_SPACE(ch)             \
                (((ch) == ' ' ||    \
                (ch) == '\f' ||     \
                (ch) == '\r' || (ch) == '\n' || (ch) == '\t'))

#define STREQU(s1, s2)      (strcmp((s1), (s2)) == 0)
#define STRIEQU(s1, s2)     (strcasecmp((s1), (s2)) == 0)

extern char* lower(char* str);
extern char* upper(char* str);

extern char* ltrim(char* str);
extern char* rtrim(char* str);
extern char* trim(char* str);

extern char* pathname(char* buf, const char* file);
extern const char* filename(const char* file);
extern char* basename(char* buf, const char* file);

#define FOUND(expr) ((expr) != -1)
extern sint find(const char* str, char ch);

#endif /* __CHARSTAR_H__ */