#include "types.h"

typedef struct SYMBOL_t {
    OBJECT_BASE base;
    char const* name;
}SYMBOL;

SYMBOL* get_symbol(char const* name);
VALUE symbol_value(char const* name);
