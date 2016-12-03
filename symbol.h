#ifndef __SYMBOL_H__
#define __SYMBOL_H__

#include "types.h"

typedef struct SYMBOL_t {
    OBJECT_BASE base;
    char const* name;
}SYMBOL;

SYMBOL* get_symbol(char const* name);
VALUE symbol_value(char const* name);

#endif
