#ifndef __FUNC_H__
#define __FUNC_H__

#include "object.h"

typedef enum {
    EXEC_NORMAL, EXEC_COMPILE, EXEC_READ
}FUNC_EXEC;

union PNODE_t;

typedef struct FUNC_t {
    OBJECT_BASE base;
    union PNODE_t* pnode;
    FUNC_EXEC exec;
}FUNC;

FUNC* create_func(union PNODE_t* pnode, OBJECT* meta);
void destroy_func(FUNC* f);
VALUE func_value(union PNODE_t* pnode, OBJECT* meta);

#endif
