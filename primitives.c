#include <mem.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

#include "vm.h"
#include "primitives.h"
#include "object.h"
#include "func.h"
#include "symbol.h"
#include "compiler.h"
#include "string.h"

const char const* primitive_names[] = {
    PRIMITIVE_FUNC_LIST(PLIST_IGNORE, PLIST_ID, PLIST_STR, PLIST_COMMA),
    PRIMITIVE_MACRO_LIST(PLIST_IGNORE, PLIST_ID, PLIST_STR, PLIST_COMMA)
};

const char const* primitive_internal_names[] = {
    PRIMITIVE_FUNC_LIST(PLIST_STR, PLIST_IGNORE, PLIST_STR, PLIST_COMMA),
    PRIMITIVE_MACRO_LIST(PLIST_STR, PLIST_IGNORE, PLIST_STR, PLIST_COMMA)
};

const int PRIMITIVE_FUNC_NUM = PRIMITIVE_FUNC_LIST(PLIST_ONE, PLIST_IGNORE, PLIST_ONE, +);
const int PRIMITIVE_MACRO_NUM = PRIMITIVE_MACRO_LIST(PLIST_ONE, PLIST_IGNORE, PLIST_ONE, +);

#define vm_assert(vm1, condition, msg) do { if(!(condition)) { vm_signal_error((vm1), (msg), __func__); return; } } while (0)

#define vm_assert_arg_not_nil(vm, val)\
do {\
    if(value_is_nil(&(val))) {\
        vm_signal_error((vm), "Argument can't be nil!", __func__);\
        return;\
    }\
}while(0)

#define vm_assert_arg_type(vm, arg, desired_type)\
do {\
    if(!((arg).type == (desired_type))) {\
        vm_signal_error((vm), "Bad argument type. Expected " #desired_type, __func__);\
        return;\
    }\
}while(0)

#define VM_PUSH_ARG(vm, x)\
do {\
    if((vm)->arg_sp < (vm)->arg_stack + ARG_STACK_SIZE)\
        *((vm)->arg_sp++) = (x);\
    else {\
        vm_signal_error((vm), "Argument stack overflow", __func__);\
        return;\
    }\
}while(0)

#define VM_POP_ARG(out, vm)\
do {\
    if((vm)->arg_sp > (vm)->arg_stack)\
        *(out) = *(--(vm)->arg_sp);\
    else {\
        vm_signal_error((vm), "Argument stack underflow", __func__);\
        return;\
    }\
}while(0)

#define VM_TPOP_ARG(out, vm, desired_type)\
do {\
    if((vm)->arg_sp > (vm)->arg_stack) {\
        *(out) = *(--(vm)->arg_sp);\
        if((out)->type != (desired_type)) {\
            vm_signal_error((vm), "Bad argument type. Expected " #desired_type, __func__);\
            return;\
        }\
    }\
    else {\
        vm_signal_error((vm), "Argument stack underflow", __func__);\
        return;\
    }\
}while(0)

void next(VM* vm) {
    ++vm->curr;
    vm->instr = vm->curr->into->fp;
}

void halt_impl(VM* vm) {
    vm->instr = NULL;
    vm->flags |= PVM_USER_HALT;
}

// ************
// * Literals *
// ************

void true_impl(VM* vm) {
    VM_PUSH_ARG(vm, PVM_TRUE);
    next(vm);
}

void false_impl(VM* vm) {
    VM_PUSH_ARG(vm, PVM_FALSE);
    next(vm);
}

void nil_impl(VM* vm) {
    VM_PUSH_ARG(vm, PVM_NIL);
    next(vm);
}

// **********************
// * Stack Manipulation *
// **********************

void push_impl(VM* vm) {
    VM_PUSH_ARG(vm, (++vm->curr)->value);
    next(vm);
}

void dup_impl(VM* vm) {
    VALUE x;
    VM_POP_ARG(&x, vm);
    VM_PUSH_ARG(vm, x);
    VM_PUSH_ARG(vm, x);
    next(vm);
}

void swap_impl(VM* vm) {
    VALUE a, b;
    VM_POP_ARG(&a, vm);
    VM_POP_ARG(&b, vm);
    VM_PUSH_ARG(vm, a);
    VM_PUSH_ARG(vm, b);
    next(vm);
}

void drop_impl(VM* vm) {
    VALUE t;
    VM_POP_ARG(&t, vm);
    next(vm);
}

// ********
// * Math *
// ********

void plus_impl(VM* vm) {
    VALUE a, b;
    VM_TPOP_ARG(&a, vm, NUM_TYPE);
    VM_TPOP_ARG(&b, vm, NUM_TYPE);
    VM_PUSH_ARG(vm, num_value(a.data.num + b.data.num));
    next(vm);
}

void minus_impl(VM* vm) {
    VALUE a, b;
    VM_TPOP_ARG(&a, vm, NUM_TYPE);
    VM_TPOP_ARG(&b, vm, NUM_TYPE);
    VM_PUSH_ARG(vm, num_value(b.data.num - a.data.num));
    next(vm);
}

void mul_impl(VM* vm) {
    VALUE a, b;
    VM_TPOP_ARG(&a, vm, NUM_TYPE);
    VM_TPOP_ARG(&b, vm, NUM_TYPE);
    VM_PUSH_ARG(vm, num_value(b.data.num * a.data.num));
    next(vm);
}


void div_impl(VM* vm) {
    VALUE a, b;
    VM_TPOP_ARG(&a, vm, NUM_TYPE);
    VM_TPOP_ARG(&b, vm, NUM_TYPE);
    VM_PUSH_ARG(vm, num_value(b.data.num / a.data.num));
    next(vm);
}

void mod_impl(VM* vm) {
    VALUE a, b;
    VM_TPOP_ARG(&a, vm, NUM_TYPE);
    VM_TPOP_ARG(&b, vm, NUM_TYPE);
    VM_PUSH_ARG(vm, num_value(fmod(b.data.num, a.data.num)));
    next(vm);
}

// **************
// * Relational *
// **************

void gt_impl(VM* vm) {
    VALUE a, b;
    VM_TPOP_ARG(&a, vm, NUM_TYPE);
    VM_TPOP_ARG(&b, vm, NUM_TYPE);
    VM_PUSH_ARG(vm, ((VALUE){.type = BOOL_TYPE, .data.boolean = b.data.num > a.data.num}));
    next(vm);
}

void lt_impl(VM* vm) {
    VALUE a, b;
    VM_TPOP_ARG(&a, vm, NUM_TYPE);
    VM_TPOP_ARG(&b, vm, NUM_TYPE);
    VM_PUSH_ARG(vm, ((VALUE){.type = BOOL_TYPE, .data.boolean = b.data.num < a.data.num}));
    next(vm);
}

void gte_impl(VM* vm) {
    VALUE a, b;
    VM_TPOP_ARG(&a, vm, NUM_TYPE);
    VM_TPOP_ARG(&b, vm, NUM_TYPE);
    VM_PUSH_ARG(vm, ((VALUE){.type = BOOL_TYPE, .data.boolean = b.data.num >= a.data.num}));
    next(vm);
}

void lte_impl(VM* vm) {
    VALUE a, b;
    VM_TPOP_ARG(&a, vm, NUM_TYPE);
    VM_TPOP_ARG(&b, vm, NUM_TYPE);
    VM_PUSH_ARG(vm, ((VALUE){.type = BOOL_TYPE, .data.boolean = b.data.num <= a.data.num}));
    next(vm);
}

void eq_impl(VM* vm) {
    VALUE a, b;
    VM_POP_ARG(&a, vm);
    VM_POP_ARG(&b, vm);
    VALUE c = {.type = BOOL_TYPE, .data.boolean = values_equal(&a, &b)};
    VM_PUSH_ARG(vm, c);
    next(vm);
}

void not_eq_impl(VM* vm) {
    VALUE a, b;
    VM_POP_ARG(&a, vm);
    VM_POP_ARG(&b, vm);
    VALUE c = {.type = BOOL_TYPE, .data.boolean = !values_equal(&a, &b)};
    VM_PUSH_ARG(vm, c);
    next(vm);
}

// *************
// * Logic Ops *
// *************

void and_impl(VM* vm) {
    VALUE a, b;
    VM_TPOP_ARG(&a, vm, BOOL_TYPE);
    VM_TPOP_ARG(&b, vm, BOOL_TYPE);
    VM_PUSH_ARG(vm, ((VALUE){.type = BOOL_TYPE, .data.boolean = b.data.boolean && a.data.boolean}));
    next(vm);
}

void or_impl(VM* vm) {
    VALUE a, b;
    VM_TPOP_ARG(&a, vm, BOOL_TYPE);
    VM_TPOP_ARG(&b, vm, BOOL_TYPE);
    VM_PUSH_ARG(vm, ((VALUE){.type = BOOL_TYPE, .data.boolean = b.data.boolean || a.data.boolean}));
    next(vm);
}

void not_impl(VM* vm) {
    VALUE a;
    VM_TPOP_ARG(&a, vm, BOOL_TYPE);
    VM_PUSH_ARG(vm, ((VALUE){.type = BOOL_TYPE, .data.boolean = !a.data.boolean}));
    next(vm);
}

// *************
// * Branching *
// *************

void jump_impl(VM* vm) {
    ++vm->curr;
	vm->curr = vm->curr->into;
	vm->instr = vm->curr->into->fp;
}

void cjump_impl(VM* vm) {
    
    VALUE c;
    VM_TPOP_ARG(&c, vm, BOOL_TYPE);
	if(c.data.boolean) {
        ++vm->curr;
	    vm->curr = vm->curr->into;
	    vm->instr = vm->curr->into->fp;
	}
	
	else {
		++vm->curr;
		next(vm);
	}
}

void exit_impl(VM* vm) {
    vm->curr = NULL;
    vm->instr = NULL;
}

// **************** 
// * Function Ops *
// ****************

void enter_impl(VM* vm) {
    vm_assert(vm, vm->ret_sp <= &vm->ret_stack[RET_STACK_SIZE - 1], "Return stack overflow");
    *vm->ret_sp = vm->curr;
    ++vm->ret_sp;
    
    vm->curr = vm->curr->into;
	next(vm);
}

void leave_impl(VM* vm) {
    vm_assert(vm, vm->ret_sp > vm->ret_stack, "Return stack underflow");
    --vm->ret_sp;
    vm->curr = *vm->ret_sp;
    next(vm);
}

void pcall_impl(VM* vm) { // call a primitive
    vm_assert(vm, vm->ret_sp <= &vm->ret_stack[RET_STACK_SIZE - 1], "Return stack overflow");
    
    VALUE v;
    VM_TPOP_ARG(&v, vm, FUNC_TYPE);
    vm->instr = ((FUNC*)v.data.obj)->pnode->fp;
}

void dcall_impl(VM* vm) { // call a function object
    vm_assert(vm, vm->ret_sp <= &vm->ret_stack[RET_STACK_SIZE - 1], "Return stack_overflow");
    *vm->ret_sp = vm->curr;
    ++vm->ret_sp;
    
    VALUE v;
    VM_TPOP_ARG(&v, vm, FUNC_TYPE);
    vm->curr = ((FUNC*)v.data.obj)->pnode;
    next(vm);
}

// **************
// * Object Ops *
// **************

void dgetf_impl(VM* vm) {
    VALUE objval;
    VM_TPOP_ARG(&objval, vm, OBJECT_TYPE);
    VALUE key;
    VM_POP_ARG(&key, vm);
    OBJECT* obj = (OBJECT*)objval.data.obj;
    VALUE val;
    map_get(&val, &obj->map, &key);
    VM_PUSH_ARG(vm, val);
    next(vm);
}

void meta_impl(VM* vm) {
    VALUE objval;
    VM_POP_ARG(&objval, vm);
    vm_assert(vm, objval.type == OBJECT_TYPE || objval.type == FUNC_TYPE, "Bad argument type. Expected OBJECT_BASE.");
	vm_assert_arg_not_nil(vm, objval);
    OBJECT* obj = (OBJECT*)objval.data.obj;
    objval.type = OBJECT_TYPE;
    objval.data.obj = (OBJECT_BASE*)obj->base.meta;
    VM_PUSH_ARG(vm, objval);
    next(vm);
}

// *********
// * Misc. *
// *********

void get_impl(VM* vm) {
    VALUE key;
    VM_POP_ARG(&key, vm);
    VALUE item;
    map_get(&item, &vm->global_scope->map, &key);
    VM_PUSH_ARG(vm, item);
    next(vm);
}

void set_impl(VM* vm) {
    VALUE key, item;
    VM_POP_ARG(&key, vm);
    
    map_put(&vm->global_scope->map, &key, &item);
    next(vm);
}

void setmac_impl(VM* vm) {
    VALUE func_val;
    VM_TPOP_ARG(&func_val, vm, FUNC_TYPE);
    vm_assert_arg_not_nil(vm, func_val);
    ((FUNC*)func_val.data.obj)->is_macro = 1;
    next(vm);
}

void macro_qm_impl(VM* vm) {
    VALUE func_val;
    VM_TPOP_ARG(&func_val, vm, FUNC_TYPE);
    vm_assert_arg_not_nil(vm, func_val);
    VM_PUSH_ARG(vm, ((VALUE){.type = BOOL_TYPE, .data.boolean = ((FUNC*)func_val.data.obj)->is_macro}));
    next(vm);
}

void type_impl(VM* vm) {
    VALUE v;
    VM_POP_ARG(&v, vm);
    
    switch(v.type) {
        case BOOL_TYPE:
            VM_PUSH_ARG(vm, symbol_value(vm, "boolean"));
            break;
        case NUM_TYPE:
            VM_PUSH_ARG(vm, symbol_value(vm, "number"));
            break;
        case FUNC_TYPE:
            VM_PUSH_ARG(vm, symbol_value(vm, "function"));
            break;
        case STRING_TYPE:
            VM_PUSH_ARG(vm, symbol_value(vm, "string"));
            break;
        case SYMBOL_TYPE:
            VM_PUSH_ARG(vm, symbol_value(vm, "symbol"));
            break;
        case OBJECT_TYPE:
            VM_PUSH_ARG(vm, symbol_value(vm, "object"));
            break;
        default:
            assert(0);
    }
    
    next(vm);
}


// ************
// * Compiler *
// ************

void program_read_impl(VM* vm) {
    VALUE v = program_read(vm);
    VM_PUSH_ARG(vm, v);
    next(vm);
}

void program_unread_impl(VM* vm) {
    VALUE v;
    VM_POP_ARG(&v, vm);
    program_unread(vm, &v);
    next(vm);
}

void compile_literal_impl(VM* vm) {
    VALUE v;
    VM_POP_ARG(&v, vm);
    compile_literal(&vm->compiler, v);
    next(vm);
}

void compile_call_impl(VM* vm) {
    VALUE func_name;
    VM_TPOP_ARG(&func_name, vm, SYMBOL_TYPE);
    VALUE func_val = lookup_by_symv(vm, &func_name);
    vm_assert_arg_type(vm, func_val, FUNC_TYPE);
    compile_call(&vm->compiler, ((FUNC*)func_val.data.obj)->pnode);
    next(vm);
}

void begin_compilation_impl(VM* vm) {
    begin_compilation(&vm->compiler);
    next(vm);
}

void end_compilation_impl(VM* vm) {
    VALUE func_name;
    VM_TPOP_ARG(&func_name, vm, SYMBOL_TYPE);
    char const* name_str = ((SYMBOL*)func_name.data.obj)->name;
    VALUE func_val = func_value(end_compilation(&vm->compiler, name_str), vm->func_meta, name_str);
    map_put(&vm->global_scope->map, &func_name, &func_val);
    next(vm);
}

void read_string_impl(VM* vm) {
    char* data = NULL;
    int len = 0;
    
    char buff[256] = {0};
    int buff_used;
    char c = 0;
    
    fgetc(vm->in);
    do {
        for(buff_used = 0; buff_used < 256 && c != '"'; ++buff_used) {
            c = fgetc(vm->in);
            vm_assert(vm, c != EOF, "Unexpected EOF");
            buff[buff_used] = c;
        }
        
        len += buff_used;
        data = realloc(data, sizeof(char) * len);
        memcpy(data + len - buff_used, buff, buff_used);
    }while(buff[buff_used - 1] != '"');
    
    data[len - 1] = 0;
    compile_literal(&vm->compiler, (VALUE){.type = STRING_TYPE, .data.obj = (OBJECT_BASE*)create_string(data, len - 1, NULL)});
    next(vm);
}

void load_impl(VM* vm) {
    VALUE fn;
    VM_TPOP_ARG(&fn, vm, STRING_TYPE);
    FILE* fp = fopen(((STRING*)fn.data.obj)->data, "r");
    FILE* old_in = vm->in;
    PNODE const* old_curr = vm->curr;
    
    vm->in = fp;

    while(!feof(vm->in)) {
        PNODE* n = pvm_compile(vm);
        if(n) {
            pvm_run(vm, n);
            free(n);
        }
        
        if(vm->flags & (PVM_RUNTIME_ERROR | PVM_COMPILE_TIME_ERROR)) {
            vm->in = old_in;
            vm->curr = old_curr;
            free(n);
            vm_assert(vm, 0, "Load aborted.");
        }
    }
    
    vm->in = old_in;
    vm->curr = old_curr;
    fclose(fp);
    next(vm);
}

void jump_macro_impl(VM* vm) {
    VM_PUSH_ARG(vm, cref_value(compile_jump(&vm->compiler), 0));
    next(vm);
}

void cjump_macro_impl(VM* vm) {
    VM_PUSH_ARG(vm, cref_value(compile_cjump(&vm->compiler), 0));
    next(vm);
}

void resolve_impl(VM* vm) {
    VALUE jnode_val;
    VM_TPOP_ARG(&jnode_val, vm, CREF_TYPE);
    resolve_jump((ANODE*)jnode_val.data.cref.ptr, compiler_pos(&vm->compiler));
    next(vm);
}

#define PLIST_PROGRAM_SET(X) {.fp = &X ## _impl}

const PNODE primitives[] = {
    PRIMITIVE_FUNC_LIST(PLIST_PROGRAM_SET, PLIST_IGNORE, PLIST_PROGRAM_SET, PLIST_COMMA),
    PRIMITIVE_MACRO_LIST(PLIST_PROGRAM_SET, PLIST_IGNORE, PLIST_PROGRAM_SET, PLIST_COMMA)
};
