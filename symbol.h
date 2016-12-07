#ifndef __SYMBOL_H__
#define __SYMBOL_H__

#include "object.h"

struct VM_t;

typedef struct SYMBOL_t {
    OBJECT_BASE base;
    char const* name;
}SYMBOL;

SYMBOL* get_symbol(struct VM_t* vm, char const* name);
VALUE symbol_value(struct VM_t* vm, char const* name);
void destroy_symbol(SYMBOL* symbol);

#endif
