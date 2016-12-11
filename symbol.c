#include "symbol.h"
#include "vm.h"
#include <malloc.h>
#include <string.h>

SYMBOL* get_symbol(VM* vm, char const* name) {
    int i;
    for(i = 0; i < vm->sym_num; ++i) {
        if(!strcmp(vm->sym_table[i]->name, name))
            return vm->sym_table[i];
    }
    
    if(vm->sym_num >= vm->sym_table_cap) {
        vm->sym_table_cap *= 2;
        vm->sym_table = realloc(vm->sym_table, sizeof(SYMBOL*) * vm->sym_table_cap);
    }
    
    SYMBOL* sym = malloc(sizeof(SYMBOL));
    sym->base.meta = vm->sym_meta;
    sym->name = strdup(name);
    vm->sym_table[vm->sym_num++] = sym;
    return sym;
}

void destroy_symbol(SYMBOL* symbol) {
    free((void*)symbol->name);
    free(symbol);
}

VALUE symbol_value(VM* vm, char const* name) {
    VALUE v;
    v.type = SYMBOL_TYPE;
    v.data.obj = (OBJECT_BASE*)get_symbol(vm, name);
    return v;
}
