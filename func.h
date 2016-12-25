#ifndef __FUNC_H__
#define __FUNC_H__

#include "object.h"

union PNODE_t;

#define PVM_FUNC_MACRO 1
#define PVM_FUNC_STUB 2
#define PVM_FUNC_DYNAMIC 4

typedef struct FUNC_t {
    OBJECT_BASE base;
    union PNODE_t const* pnode;
    char const* name;
    int flags;
}FUNC;

FUNC* create_func(union PNODE_t const* pnode, OBJECT* meta, char const* name, int flags);
void destroy_func(FUNC* f);
VALUE func_value(union PNODE_t const* pnode, OBJECT* meta, char const* name, int flags);

#endif
