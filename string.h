#ifndef __STRING_H__
#define __STRING_H__

#include "object.h"

typedef struct {
    OBJECT_BASE base;
    char const* data;
    int len;
}STRING;

STRING* create_string(char const* data, int len, OBJECT* meta);
void destroy_string(STRING* s);

#endif
