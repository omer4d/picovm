#ifndef __VM_H__
#define __VM_H__

#include <stdio.h>

#include "pnode.h"
#include "value.h"
#include "object.h"
#include "compiler.h"

#define ARG_STACK_SIZE 1024
#define RET_STACK_SIZE 1024
#define READ_BUFF_SIZE 64

struct SYMBOL_t;
struct OBJECT_t;
struct FUNC_t;

#define PVM_RUNTIME_ERROR 1
#define PVM_COMPILE_TIME_ERROR 2
#define PVM_USER_HALT 4

typedef struct {
    PNODE const* ret_stack[RET_STACK_SIZE];
    PNODE const** ret_sp;
    PNODE const* curr;
}VM_CONTINUATION_DATA;

typedef struct {
    VALUE* arg_stack;
    VALUE* arg_sp;
    
    PNODE const** ret_stack;
    PNODE const** ret_sp;
    
    PNODE const* curr;
}VM_EXECUTION_CONTEXT;

typedef struct VM_t {
    FILE* log_stream;
    FILE* in;
    
    int flags;
    
    VALUE read_buff[READ_BUFF_SIZE];
    int read_queue_start, read_queue_end;
    
    VALUE arg_stack_data[ARG_STACK_SIZE];
    PNODE const* ret_stack_data[RET_STACK_SIZE];
    
    VM_EXECUTION_CONTEXT xc;
    
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
VALUE lookup_by_name(VM* vm, char const* name);
VALUE lookup_by_symv(VM* vm, VALUE const* sym);
void set_method(VM* vm, OBJECT* object, char const* name, struct FUNC_t* func);

PNODE* pvm_compile(VM* vm);
// Reads from the input stream and compiles until one of the following occurs:
// (1) A newline is reached
// (2) EOF is reached
// (3) An error is signalled
// Return value: a pointer to the beginning of the compiled (sub)program
// Side-effects:
// - Input stream position: after the last character read
// - Flags: sets or clears the compile time error flag. Otherwise unchanged.
// - Read Queue: undefined
// - Argument stack: unchanged
// - Continuation: unchanged
// - Global scope: undefined
// - Compiler state: unuchanged


void pvm_run(VM* vm, PNODE* pnode);
// Runs the provided function.
// Side-effects:
// - Input stream position: after the last character read
// - Flags: may set the compile time error flag. Otherwise unchanged.
// - Read Queue: undefined
// - Compile-time argument stack: undefined
// - Runtime argument stack: unchanged
// - Continuation: unchanged
// - Global scope: undefined
// - Compiler: unuchanged



void pvm_resume(VM* vm);
int pvm_test_flags(VM* vm, int f);
void pvm_set_flags(VM* vm, int f);
void pvm_clear_flags(VM* vm, int f);

void pvm_get_cc(VM_CONTINUATION_DATA* cc, VM* vm);

VALUE program_read(VM* vm);
void program_unread(VM* vm, VALUE const* v); 

void vm_signal_error(VM* vm, char const* msg, char const* primitive);

#endif
