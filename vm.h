#ifndef __VM_H__
#define __VM_H__

#include "value.h"
#include "types.h"

#define ARG_STACK_SIZE 1024
#define RET_STACK_SIZE 1024
#define MARK_STACK_SIZE 256

struct VM_t;

typedef void (*CFUN)(struct VM_t*);

typedef union PNODE_t {
    union PNODE_t* into;
    CFUN fp;
    VALUE value;
}PNODE;

typedef struct VM_t {
    PNODE* program;
    char const** debug_info;
    PNODE* program_pos;

    VALUE* arg_stack;
    VALUE* arg_sp;

    PNODE** ret_stack;
    PNODE** ret_sp;

    PNODE* mark_stack;
    PNODE* mark_sp;

    PNODE* curr;
    CFUN instr;
    
    OBJECT* global_scope;
}VM;

typedef struct FUNC_t {
    OBJECT_BASE base;
    union PNODE_t* pnode;
}FUNC;

VM* create_vm();
void destroy_vm(VM* vm);
void push(VM* vm, VALUE x);
VALUE pop(VM* vm);

#endif
