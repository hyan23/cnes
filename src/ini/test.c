// test.c
// Author: hyan23
// Date: 2018.05.28

#include <stdio.h>
#include "header.h"
#include "ini.h"

int main(void)
{
    ini_file_s ini;
    ini_open(&ini, "test.ini");
    ini_append(&ini, "section2", "property", "value");
    ini_dump(stdout, &ini);
    char string[BUFFER_MIDDLE];
    BOOL bl = FALSE;
    double numeric = .0;
    ini_string(&ini, "section1", "abc", string);
    printf("section1.abc=%s\n", string);
    ini_string(&ini, "profile", "birthday", string);
    printf("profile.birthday=%s\n", string);
    ini_numeric(&ini, "section1", "pi", &numeric);
    printf("section1.pi=%lf\n", numeric);
    ini_boolean(&ini, "profile", "guy", &bl);
    printf("profile.guy=%d\n", bl);
    ini_close(&ini);
    return 0;
}