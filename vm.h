#ifndef __VM_H__
#define __VM_H__

#include "tokenizer.h"
#include "value.h"
#include "object.h"

#define ARG_STACK_SIZE 1024
#define RET_STACK_SIZE 1024
#define MARK_STACK_SIZE 256

struct SYMBOL_t;
struct OBJECT_t;

struct VM_t;

typedef void (*CFUN)(struct VM_t*);

typedef union PNODE_t {
    union PNODE_t* into;
    CFUN fp;
    VALUE value;
}PNODE;

typedef struct VM_t {
    FILE* in;
    PNODE* program;
    char const** debug_info;
    PNODE* program_write_start;
    PNODE* program_write_pos;

    VALUE* arg_stack;
    VALUE* arg_sp;

    PNODE** ret_stack;
    PNODE** ret_sp;

    PNODE* mark_stack;
    PNODE* mark_sp;

    PNODE* curr;
    CFUN instr;
    
    struct SYMBOL_t** sym_table;
    int sym_table_cap;
    int sym_num;
    
    struct OBJECT_t* default_meta;
    struct OBJECT_t* sym_meta;
    struct OBJECT_t* primitive_func_meta;
    struct OBJECT_t* func_meta;
    
    struct OBJECT_t* global_scope;
}VM;

VM* create_vm();
void destroy_vm(VM* vm);
void push(VM* vm, VALUE x);
VALUE pop(VM* vm);
void eval_str(VM* vm, FILE* stream);

#endif
