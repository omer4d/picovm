#include "func.h"
#include "vm.h"

FUNC* create_func(PNODE const* pnode, OBJECT* meta) {
    FUNC* f = malloc(sizeof(FUNC));
    
    f->base.meta = meta;
    f->pnode = pnode;
    f->is_macro = 0;
    
    return f;
}

void destroy_func(FUNC* f) {
    free(f);
}

VALUE func_value(PNODE const* pnode, OBJECT* meta) {
    VALUE v;
    v.type = FUNC_TYPE;
    v.data.obj = (OBJECT_BASE*)create_func(pnode, meta);
    return v;
}
