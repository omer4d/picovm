#ifndef __PNODE_H__
#define __PNODE_H__

#include "value.h"

struct VM_t;

typedef void (*CFUN)(struct VM_t*);

typedef union PNODE_t {
    union PNODE_t const* into;
    CFUN fp;
    VALUE value;
}PNODE;

#endif
