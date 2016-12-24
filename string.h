#ifndef __STRING_H__
#define __STRING_H__

#include "object.h"

typedef struct STRING_t {
    OBJECT_BASE base;
    char const* data;
    int len;
}STRING;

void init_string(STRING* str, char const* data, int len, OBJECT* meta);
STRING* create_string(char const* data, int len, OBJECT* meta);
void destroy_string(STRING* s);
int strings_equal(STRING const* a, STRING const* b);

#endif
