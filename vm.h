#ifndef __VM_H__
#define __VM_H__

#include <stdio.h>

#include "pnode.h"
#include "value.h"
#include "object.h"
#include "compiler.h"

#define ARG_STACK_SIZE 1024
#define RET_STACK_SIZE 1024

struct SYMBOL_t;
struct OBJECT_t;
struct FUNC_t;

typedef struct VM_t {
    FILE* log_stream;
    
    VALUE arg_stack[ARG_STACK_SIZE];
    VALUE* arg_sp;
    
    PNODE const* ret_stack[RET_STACK_SIZE];
    PNODE const** ret_sp;
    
    PNODE const* curr;
    CFUN instr;
    
    struct SYMBOL_t** sym_table;
    int sym_table_cap;
    int sym_num;
    
    struct OBJECT_t* default_meta;
    struct OBJECT_t* sym_meta;
    struct OBJECT_t* primitive_func_meta;
    struct OBJECT_t* func_meta;
    
    struct OBJECT_t* global_scope;
    
    COMPILER compiler;
}VM;

VM* create_vm();
void destroy_vm(VM* vm);
void push(VM* vm, VALUE x);
VALUE pop(VM* vm);
void print_debug_info(VM* vm);
char* value_to_string(char* str, VALUE* sp);
PNODE const* register_func(VM* vm, PNODE const* pnode, char const* name, int primitive);
PNODE const* register_macro(VM* vm, PNODE const* pnode, char const* name, int primitive);
VALUE lookup(VM* vm, char const* name);
void set_method(VM* vm, OBJECT* object, char const* name, struct FUNC_t* func);
void pvm_eval(VM* vm);

VALUE program_read(VM* c);

#endif
