// charstar.c
// Author: hyan23
// Date: 2017.08.02

#include "charstar.h"

char* lower(char* str)
{
    char* temp = str;
    while (*temp) {
        if (*temp >= 'A' && *temp <= 'Z') {
            *temp += 'a' - 'A';
        }
        temp ++;
    }
    return str;
}

char* upper(char* str)
{
    char* temp = str;
    while (*temp) {
        if (*temp >= 'a' && *temp <= 'z') {
            *temp -= 'a' - 'A';
        }
        temp ++;
    }
    return str;
}

char* ltrim(char* str)
{
    const char* temp = str;
    while (*temp) {
        if (!WHITE_SPACE(*temp)) {
            break;
        }
        temp ++;
    }
    if (*temp == '\0') {
        *str = '\0';
    } else if (temp != str) {
        strcpy(str, temp);
    }
    return str;
}

char* rtrim(char* str)
{
    uint length = strlen(str);
    for (uint i = length - 1; i > 0; i --) {
        if (!WHITE_SPACE(str[i])) {
            break;
        }
        str[i] = '\0';
    }
    return str;
}

char* trim(char* str)
{
    return ltrim(str), rtrim(str);
}

char* pathname(char* buf, const char* file)
{
    strcpy(buf, file);
    sint i = strlen(file);
    while (-- i >= 0) {
        if (buf[i] == '/') {
            break;
        }
        buf[i] = '\0';
    }
    return buf;
}

const char* filename(const char* file)
{
    const char* temp = file, *slash = file;
    while (*temp) {
        if (*temp == '/') {
            slash = temp;
        }
        temp ++;
    }
    return slash + 1;
}

char* basename(char* buf, const char* file)
{
    strcpy(buf, filename(file));
    const char* temp = buf;
    while (*temp) {
        if (*temp == '.') {
            break;
        }
        temp ++;
    }
    buf[temp - buf] = '\0';
    return buf;
}

sint find(const char* str, char ch)
{
    const char* temp = str;
    while (*temp) {
        if (*temp == ch) {
            return temp - str;
        }
        temp ++;
    }
    return -1;
}