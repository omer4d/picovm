#include "func.h"
#include "vm.h"
#include <malloc.h>

FUNC* create_func(PNODE const* pnode, OBJECT* meta, char const* name) {
    FUNC* f = malloc(sizeof(FUNC));
    
    f->base.meta = meta;
    f->pnode = pnode;
    f->is_macro = 0;
    f->name = name;
    
    return f;
}

void destroy_func(FUNC* f) {
    free(f);
}

VALUE func_value(PNODE const* pnode, OBJECT* meta, char const* name) {
    VALUE v;
    v.type = FUNC_TYPE;
    v.data.obj = (OBJECT_BASE*)create_func(pnode, meta, name);
    return v;
}
