#ifndef __FUNC_H__
#define __FUNC_H__

#include "object.h"

union PNODE_t;

typedef struct FUNC_t {
    OBJECT_BASE base;
    union PNODE_t* pnode;
}FUNC;

FUNC* create_func(union PNODE_t* pnode, OBJECT* meta);
void destroy_func(FUNC* f);
VALUE func_value(union PNODE_t* pnode, OBJECT* meta);

#endif