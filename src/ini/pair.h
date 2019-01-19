// pair.h
// Author: hyan23
// Date: 2018.05.28

#ifndef __PAIR_H__
#define __PAIR_H__

#include "header.h"

#define INI_PAIR_COMMENT_LEN        (BUFFER_MIDDLE - 1)
#define INI_PAIR_NAME_LEN           (4 * BUFFER_SMALL - 1)
#define INI_PAIR_VAL_LEN            (4 * BUFFER_SMALL - 1)

typedef struct INI_PAIR {
    char comment[INI_PAIR_COMMENT_LEN + 1];
    uint clength;   /* comment length */
    char name[INI_PAIR_NAME_LEN + 1];
    char value[INI_PAIR_VAL_LEN + 1];
}
ini_pair_s;

// necessary
extern void ini_pair_init(ini_pair_s* pair, const char* name, const char* value);
// append a comment line to pair
// 'comment' can contain multiple lines which separated by a '\n' character, 
// i.e. <comment line 1>'\n'<comment line 2>'\n'...<comment line n>
extern error_t ini_pair_cmt(ini_pair_s* pair, const char* comment);
// # comment line 1
// # comment line 2
// # comment line 3
// name=value
extern void ini_pair_dump(FILE* fout, const ini_pair_s* pair);

#endif /* __PAIR_H__ */