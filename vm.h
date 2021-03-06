#ifndef __VM_H__
#define __VM_H__

#include <stdio.h>

#include "charstream.h"
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
    PNODE const* ret_stack_data[RET_STACK_SIZE];
    PNODE const** ret_stack;
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
    CHARSTREAM* in;
    
    int flags;
    
    VALUE read_buff[READ_BUFF_SIZE];
    int read_queue_start, read_queue_end;
    
    VALUE arg_stack_data[ARG_STACK_SIZE];
    PNODE const* ret_stack_data[RET_STACK_SIZE];
    
    VM_EXECUTION_CONTEXT xc;
    
    CFUN instr;
    
    struct SYMBOL_t** sym_table;
    MAP string_table;
    
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
void pvm_trace(VM* vm);
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
// - Compiler state: unchanged.

void pvm_run(VM* vm, PNODE* pnode);
// Runs the provided function.
// Side-effects:
// - Input stream position: unchanged
// - Flags: sets or clears the runtime time error flag. Otherwise unchanged.
// - Read Queue: undefined
// - Argument stack: undefined
// - Continuation: undefined
// - Global scope: undefined
// - Compiler state: unchanged.



VM_EXECUTION_CONTEXT pvm_protect_xc(VM* vm);
void pvm_restore_xc(VM* vm, VM_EXECUTION_CONTEXT const* xc);

void pvm_resume(VM* vm);
void pvm_exec(VM* vm, VALUE v);
int pvm_test_flags(VM* vm, int f);
void pvm_set_flags(VM* vm, int f);
void pvm_clear_flags(VM* vm, int f);
void pvm_trace(VM* vm);

void pvm_get_cc(VM_CONTINUATION_DATA* cc, VM* vm);
void pvm_continue(VM* vm, VM_CONTINUATION_DATA* cc);

VALUE program_read(VM* vm);
void program_unread(VM* vm, VALUE const* v); 

void vm_signal_error(VM* vm, char const* msg, char const* primitive);
void vm_signal_silent_error(VM* vm);
void vm_log(VM* vm, char const *fmt, ...);

#endif
