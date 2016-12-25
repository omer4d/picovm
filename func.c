#include "func.h"
#include "vm.h"
#include <malloc.h>

FUNC* create_func(PNODE const* pnode, OBJECT* meta, char const* name, int flags) {
    FUNC* f = malloc(sizeof(FUNC));
    
    f->base.meta = meta;
    f->pnode = pnode;
    f->flags = flags;
    f->name = name;
    
    return f;
}

void destroy_func(FUNC* f) {
    free(f);
}

VALUE func_value(PNODE const* pnode, OBJECT* meta, char const* name, int flags) {
    VALUE v;
    v.type = FUNC_TYPE;
    v.data.obj = (OBJECT_BASE*)create_func(pnode, meta, name, flags);
    return v;
}
