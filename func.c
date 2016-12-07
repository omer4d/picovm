#include "func.h"
#include "vm.h"

FUNC* create_func(PNODE* pnode, OBJECT* meta) {
    FUNC* f = malloc(sizeof(FUNC));
    
    f->base.meta = meta;
    f->pnode = pnode;
    f->exec = EXEC_NORMAL;
    
    return f;
}

void destroy_func(FUNC* f) {
    free(f);
}

VALUE func_value(PNODE* pnode, OBJECT* meta) {
    VALUE v;
    v.type = FUNC_TYPE;
    v.data.obj = (OBJECT_BASE*)create_func(pnode, meta);
    return v;
}
