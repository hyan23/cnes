// pair.c
// Author: hyan23
// Date: 2018.05.28

#include "header.h"
#include "pair.h"

void 
ini_pair_init(ini_pair_s* pair, 
const char* name, const char* value)
{
    CLEAR(pair);
    assert(NOTNULL(name) && NOTNULL(value));
    strncpy(pair->name, name, INI_PAIR_NAME_LEN + 1);
    strncpy(pair->value, value, INI_PAIR_VAL_LEN + 1);
    pair->clength = 0;
}

error_t ini_pair_cmt(ini_pair_s* pair, const char* comment)
{
                                            /*  '\n'    */
    uint length = pair->clength + strlen(comment) + 1;
    if (length > INI_PAIR_COMMENT_LEN) {
        return MEMORY_OVERFLOW;
    }
    strcat(pair->comment, comment);
    pair->clength = length;
    pair->comment[pair->clength - 1] = '\n';
    return SUCCESS;
}

void ini_pair_dump(FILE* fout, const ini_pair_s* pair)
{
    char line[INI_PAIR_COMMENT_LEN + 1];
    uint pos = 0, n = 0;
    while (sscanf(pair->comment + pos, "%[^\n]%n", line, &n) == 1) {
        fprintf(fout, "#%s\n", line);
        pos += n + 1;
    }
    fprintf(fout, "%s=%s\n", pair->name, pair->value);
}