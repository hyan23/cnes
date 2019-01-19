// ini.c
// Author: hyan23
// Date: 2018.05.27

#include "header.h"
#include "charstar.h"
#include "ini.h"

static sint find_section(ini_section_s* section, const char* sname)
{
    return STRIEQU(section->sname, sname) ? ARRAY_CMP_EQUAL : -1;
}

static ini_section_s* 
ini_section(ini_file_s* ini, const char* sname)
{
    if (NUL(sname)) {
        sname = INI_SECTION_NAME;
    }
    return array_find(&ini->sections, sname, (array_cmp_func) find_section);
}

static error_t read_sections(ini_file_s* ini)
{
    ini_section_s global;
    if (FAILED(ini_section_init(&global, INI_SECTION_NAME))) {
        return MEMORY_OVERFLOW;
    }
    // current section
    ini_section_s* section = array_append(&ini->sections, &global);
    if (NUL(section)) {
        ini_section_destory(&global);
        return MEMORY_OVERFLOW;
    }
    char* buffer = NULL;
    ini_pair_s pair;    /* store comments of next pair */
    while (fscanf(ini->fin, " %m[^\n]\n", &buffer) != EOF) {
      // buffer == NULL indicates blank lines
      if (NOTNULL(buffer)) {
        const char* trimmed = rtrim(buffer);
        if (COMMENT_SIGN(trimmed[0])) {
          ini_pair_cmt(&pair, trimmed + 1);
        } else if (SECTION_SIGN(trimmed[0])) {
          char* s1 = NULL;
          if (sscanf(buffer, "[ %m[^]]", &s1) != EOF) {
            if (NOTNULL(s1)) {
              const char* sname = rtrim(s1);
              ini_section_s* temp = ini_section(ini, sname);
              if (NOTNULL(temp)) {
                section = temp;
              } else {
                ini_section_s temp;
                if (FAILED(ini_section_init(&temp, sname))) {
                  FREE(buffer);
                  return MEMORY_OVERFLOW;
                }
                section = (ini_section_s*) array_append(&ini->sections, 
                  &temp);
                if (NUL(section)) {
                  FREE(buffer);
                  return MEMORY_OVERFLOW;
                }
              }
              FREE(s1);
            }
          }
        } else {
          char* s1 = NULL, *s2 = NULL;
          if (sscanf(trimmed, "%m[^=]= %m[^\n]", &s1, &s2) != EOF) {
            if (NOTNULL(s1) && NOTNULL(s2)) {
              const char* name = rtrim(s1), *value = rtrim(s2);
              ini_pair_s temp;
              ini_pair_init(&temp, name, value);
              ini_pair_cmt(&temp, pair.comment);
              ini_pair_init(&pair, name, value);  /* clear comments */
              if (NUL(ini_section_append(section, &temp))) {
                FREE(buffer);
                return MEMORY_OVERFLOW;
              }
            }
            FREE(s1);
            FREE(s2);
          }
        }
        FREE(buffer);
      }
    }
    return SUCCESS;
}

error_t ini_open(ini_file_s* ini, const char* filename)
{
    CLEAR(ini);
    if (FAILED(array_create(&ini->sections, ini_section_s, 
        INI_SECTION_NUMBER, INI_SECTION_INC))) {
        return ini->result = MEMORY_OVERFLOW;
    }
    if (NOTNULL(filename)) {
        strncpy(ini->file, filename, FILENAME);
        ini->fin = fopen(ini->file, "r");
        if (NUL(ini->fin)) {
            ini_close(ini);
            return ini->result = OPEN_FILE_ERROR;
        }
        error_t result = read_sections(ini);
        if (FAILED(result)) {
            ini_close(ini);
            return ini->result = result;
        }
    }
    return ini->result = SUCCESS;
}

void ini_close(ini_file_s* ini)
{
    ini_remove(ini, NULL, NULL);
    if (NOTNULL(ini->fin)) {
        fclose(ini->fin);
    }
}

void ini_dump(FILE* fout, const ini_file_s* ini)
{
    array_dump(fout, &ini->sections, 
        (array_dump_func) ini_section_dump);
}

ini_pair_s* 
ini_pair(ini_file_s* ini, 
const char* sname, const char* property)
{
    if (NUL(sname)) {
        sname = INI_SECTION_NAME;
    }
    ini_section_s* section = array_find(&ini->sections, 
        sname, (array_cmp_func) find_section);
    if (NOTNULL(section)) {
        ini_pair_s* pair = ini_section_pair(section, property);
        ini->result = NOTNULL(pair) ? SUCCESS : NOTEXIST;
        return pair;
    }
    ini->result = NOTEXIST;
    return NULL;
}

ini_pair_s* 
ini_append(ini_file_s* ini, 
const char* sname, const char* property, const char* value)
{
    ini_section_s* section = ini_section(ini, sname);
    if (NUL(section)) {
        ini_section_s temp;
        if (FAILED(ini_section_init(&temp, sname))) {
            ini->result = MEMORY_OVERFLOW;
            return NULL;
        }
        section = array_append(&ini->sections, &temp);
        if (NUL(section)) {
            ini_section_destory(&temp);
            ini->result = MEMORY_OVERFLOW;
            return NULL;
        }
    }
    ini_pair_s* pair = ini_pair(ini, sname, property);
    if (NOTNULL(pair)) {
        ini->result = EXISTS;
        return NULL;
    }
    ini_pair_s temp;
    ini_pair_init(&temp, property, value);
    pair = ini_section_append(section, &temp);
    if (NUL(pair)) {
        ini_remove(ini, sname, NULL);
        ini->result = MEMORY_OVERFLOW;
        return NULL;
    }
    ini->result = SUCCESS;
    return pair;
}

void ini_remove(ini_file_s* ini, const char* sname, const char* property)
{
    if (NUL(sname) && NUL(property)) {
        array_destroy(&ini->sections, (array_destroy_func) ini_section_destory);
    } else {
        if (NUL(property)) {
            array_remove(&ini->sections, sname, (array_cmp_func) find_section);
        } else {
            ini_section_s* section = ini_section(ini, sname);
            if (NOTNULL(section)) {
                ini_section_remove(section, property);
            }
        }
    }
}

error_t 
ini_string(ini_file_s* ini, 
const char* sname, const char* property, char* string)
{
    const ini_pair_s* pair = ini_pair(ini, sname, property);
    if (NOTNULL(pair)) {
        strcpy(string, pair->value);
        return ini->result = SUCCESS;
    }
    return ini->result = NOTEXIST;
}

error_t 
ini_numeric(ini_file_s* ini, 
const char* sname, const char* property, double* value)
{
    const ini_pair_s* pair = ini_pair(ini, sname, property);
    if (NOTNULL(pair)) {
        double temp = .0;
        if (sscanf(pair->value, "%lf", &temp) == 1) {
            *value = temp;
            return ini->result = SUCCESS;
        }
        return ini->result = FAILURE;
    }
    return ini->result = NOTEXIST;
}

error_t 
ini_boolean(ini_file_s* ini, 
const char* sname, const char* property, BOOL* value)
{
    const ini_pair_s* pair = ini_pair(ini, sname, property);
    if (NOTNULL(pair)) {
        sint temp = 0;
        if (sscanf(pair->value, "%d", &temp) == 1) {
            *value = temp != 0 ? TRUE : FALSE;
            return ini->result = SUCCESS;
        }
        if (STRIEQU(pair->value, "true")) {
            *value = TRUE;
            return ini->result = SUCCESS;
        } else if (STRIEQU(pair->value, "false")) {
            *value = FALSE;
            return ini->result = SUCCESS;
        }
        return ini->result = FAILURE;
    }
    return ini->result = NOTEXIST;
}

const char* ini_member(const char* expr)
{
    uint pos = 0, old = 0;
    const char* temp = expr;
    while (*temp) {
        if (MEMBER_SEL_OPERATOR(*temp)) {
            old = pos;
            pos = temp - expr;
        }
        temp ++;
    }
    return expr + old + 1;
}

error_t 
member_selction(ini_file_s* ini, 
char (*sname)[INI_SECTION_NAME_LEN + 1], char (*property)[INI_PAIR_NAME_LEN + 1], 
const char* member)
{
    CLEAR(sname);
    CLEAR(property);
    char FORMAT[BUFFER_MIDDLE];
    sprintf(FORMAT, "%%[^%s]%%*[%s]%%[^\n]", 
        MEMBER_SEL_OPERATORS, MEMBER_SEL_OPERATORS);
    // puts(FORMAT);
    if (sscanf(member, FORMAT, *sname, *property) == 2) {
        return ini->result = SUCCESS;
    }
    return ini->result = ARGUMENTS;
}