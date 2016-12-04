#include "vm.h"
#include <malloc.h>

OBJECT* sym_meta = NULL;

SYMBOL* get_symbol(OBJECT* default_meta, char const* name) {
    static SYMBOL** sym_table = NULL;
    static int sym_table_cap = 0;
    static int sym_num = 0;
    
    if(!sym_table) {
        sym_table_cap = 8;
        sym_table = malloc(sizeof(SYMBOL*) * sym_table_cap);
    }
    
    if(!sym_meta) {
        sym_meta = create_object(default_meta);
    }
    
    int i;
    for(i = 0; i < sym_num; ++i) {
        if(!strcmp(sym_table[i]->name, name))
            return sym_table[i];
    }
    
    if(sym_num >= sym_table_cap) {
        sym_table_cap *= 2;
        sym_table = realloc(sym_table, sizeof(SYMBOL*) * sym_table_cap);
    }
    
    SYMBOL* sym = malloc(sizeof(SYMBOL));
    sym->base.meta = sym_meta;
    sym->name = name;
    sym_table[sym_num++] = sym;
    return sym;
}

VALUE symbol_value(char const* name) {
    VALUE v;
    v.type = SYMBOL_TYPE;
    v.data.obj = (OBJECT_BASE*)get_symbol(name);
    return v;
}
