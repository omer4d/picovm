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
    PRIMITIVE_FUNC_LIST(PLIST_IGNORE, PLIST_STR, PLIST_STR, PLIST_COMMA),
    PRIMITIVE_MACRO_LIST(PLIST_IGNORE, PLIST_STR, PLIST_STR, PLIST_COMMA)
};

const char const* primitive_internal_names[] = {
    PRIMITIVE_FUNC_LIST(PLIST_STR, PLIST_IGNORE, PLIST_STR, PLIST_COMMA),
    PRIMITIVE_MACRO_LIST(PLIST_STR, PLIST_IGNORE, PLIST_STR, PLIST_COMMA)
};

const int PRIMITIVE_FUNC_NUM = PRIMITIVE_FUNC_LIST(PLIST_ONE, PLIST_IGNORE, PLIST_ONE, +);
const int PRIMITIVE_MACRO_NUM = PRIMITIVE_MACRO_LIST(PLIST_ONE, PLIST_IGNORE, PLIST_ONE, +);

void next(VM* vm) {
    ++vm->curr;
    vm->instr = vm->curr->into->fp;
}

#define vm_assert(vm, condition, msg) do { if(!(condition)) { vm_signal_error((vm), (msg), __func__); return; } while (0)

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
    VALUE tmp;
    VM_POP_ARG(&tmp, vm);
    next(vm);
}

// ********
// * Math *
// ********

void plus_impl(VM* vm) {
    VALUE a, b;
    VM_POP_ARG(&a, vm);
    VM_POP_ARG(&b, vm);
    assert(a.type == NUM_TYPE && b.type == NUM_TYPE);
    VM_PUSH_ARG(vm, num_value(a.data.num + b.data.num));
    next(vm);
}

void minus_impl(VM* vm) {
    VALUE a, b;
    VM_POP_ARG(&a, vm);
    VM_POP_ARG(&b, vm);

    assert(a.type == NUM_TYPE && b.type == NUM_TYPE);
    VM_PUSH_ARG(vm, num_value(b.data.num - a.data.num));
    next(vm);
}

void mul_impl(VM* vm) {
    VALUE a, b;
    VM_POP_ARG(&a, vm);
    VM_POP_ARG(&b, vm);

    assert(a.type == NUM_TYPE && b.type == NUM_TYPE);
    VM_PUSH_ARG(vm, num_value(b.data.num * a.data.num));
    next(vm);
}


void div_impl(VM* vm) {
    VALUE a, b;
    VM_POP_ARG(&a, vm);
    VM_POP_ARG(&b, vm);

    assert(a.type == NUM_TYPE && b.type == NUM_TYPE);
    VM_PUSH_ARG(vm, num_value(b.data.num / a.data.num));
    next(vm);
}

void mod_impl(VM* vm) {
    VALUE a, b;
    VM_POP_ARG(&a, vm);
    VM_POP_ARG(&b, vm);

    assert(a.type == NUM_TYPE && b.type == NUM_TYPE);
    VM_PUSH_ARG(vm, num_value(fmod(b.data.num, a.data.num)));
    next(vm);
}

// **************
// * Relational *
// **************

void gt_impl(VM* vm) {
    VALUE a, b;
    VM_POP_ARG(&a, vm);
    VM_POP_ARG(&b, vm);

    assert(a.type == NUM_TYPE && b.type == NUM_TYPE);
    VM_PUSH_ARG(vm, ((VALUE){.type = BOOL_TYPE, .data.boolean = b.data.num > a.data.num}));
    next(vm);
}

void lt_impl(VM* vm) {
    VALUE a, b;
    VM_POP_ARG(&a, vm);
    VM_POP_ARG(&b, vm);

    assert(a.type == NUM_TYPE && b.type == NUM_TYPE);
    VM_PUSH_ARG(vm, ((VALUE){.type = BOOL_TYPE, .data.boolean = b.data.num < a.data.num}));
    next(vm);
}

void gte_impl(VM* vm) {
    VALUE a, b;
    VM_POP_ARG(&a, vm);
    VM_POP_ARG(&b, vm);

    assert(a.type == NUM_TYPE && b.type == NUM_TYPE);
    VM_PUSH_ARG(vm, ((VALUE){.type = BOOL_TYPE, .data.boolean = b.data.num >= a.data.num}));
    next(vm);
}

void lte_impl(VM* vm) {
    VALUE a, b;
    VM_POP_ARG(&a, vm);
    VM_POP_ARG(&b, vm);

    assert(a.type == NUM_TYPE && b.type == NUM_TYPE);
    VM_PUSH_ARG(vm, ((VALUE){.type = BOOL_TYPE, .data.boolean = b.data.num <= a.data.num}));
    next(vm);
}

void eq_impl(VM* vm) {
    VALUE a;
    VM_POP_ARG(&a, vm);
    VALUE b;
    VM_POP_ARG(&b, vm);
    VALUE c = {.type = BOOL_TYPE, .data.boolean = values_equal(&a, &b)};
    VM_PUSH_ARG(vm, c);
    next(vm);
}

void not_eq_impl(VM* vm) {
    VALUE a;
    VM_POP_ARG(&a, vm);
    VALUE b;
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
    VM_POP_ARG(&a, vm);
    VM_POP_ARG(&b, vm);

    assert(a.type == BOOL_TYPE && b.type == BOOL_TYPE);
    VM_PUSH_ARG(vm, ((VALUE){.type = BOOL_TYPE, .data.boolean = b.data.boolean && a.data.boolean}));
    next(vm);
}

void or_impl(VM* vm) {
    VALUE a, b;
    VM_POP_ARG(&a, vm);
    VM_POP_ARG(&b, vm);

    assert(a.type == BOOL_TYPE && b.type == BOOL_TYPE);
    VM_PUSH_ARG(vm, ((VALUE){.type = BOOL_TYPE, .data.boolean = b.data.boolean || a.data.boolean}));
    next(vm);
}

void not_impl(VM* vm) {
    VALUE a;
    VM_POP_ARG(&a, vm);
    assert(a.type == BOOL_TYPE);
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
    VM_POP_ARG(&c, vm);
    assert(c.type == BOOL_TYPE);
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
    assert(vm->ret_sp <= &vm->ret_stack[RET_STACK_SIZE - 1]);
    *vm->ret_sp = vm->curr;
    ++vm->ret_sp;
    
    vm->curr = vm->curr->into;
	next(vm);
}

void leave_impl(VM* vm) {
    assert(vm->ret_sp > vm->ret_stack);
    --vm->ret_sp;
    vm->curr = *vm->ret_sp;
    next(vm);
}

void pcall_impl(VM* vm) { // call a primitive
    assert(vm->ret_sp <= &vm->ret_stack[RET_STACK_SIZE - 1]);
    VALUE v;
    VM_POP_ARG(&v, vm);
    assert(v.type == FUNC_TYPE);
    vm->instr = ((FUNC*)v.data.obj)->pnode->fp;
}

void dcall_impl(VM* vm) { // call a function object
    assert(vm->ret_sp <= &vm->ret_stack[RET_STACK_SIZE - 1]);
    *vm->ret_sp = vm->curr;
    ++vm->ret_sp;
    VALUE v;
    VM_POP_ARG(&v, vm);
    assert(v.type == FUNC_TYPE);
    vm->curr = ((FUNC*)v.data.obj)->pnode;
    next(vm);
}

// **************
// * Object Ops *
// **************

void dgetf_impl(VM* vm) {
    VALUE objval;
    VM_POP_ARG(&objval, vm);
    VALUE key;
    VM_POP_ARG(&key, vm);
    assert(objval.type == OBJECT_TYPE);
    OBJECT* obj = (OBJECT*)objval.data.obj;
    VALUE val;
    map_get(&val, &obj->map, &key);
    VM_PUSH_ARG(vm, val);
    next(vm);
}

void meta_impl(VM* vm) {
    VALUE objval;
    VM_POP_ARG(&objval, vm);
    assert(objval.type == OBJECT_TYPE || objval.type == FUNC_TYPE);
    assert(!value_is_nil(&objval));
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
    VALUE item;
    VM_POP_ARG(&item, vm);
    VALUE key;
    VM_POP_ARG(&key, vm);
    map_put(&vm->global_scope->map, &key, &item);
    next(vm);
}

void setmac_impl(VM* vm) {
    VALUE func_val;
    VM_POP_ARG(&func_val, vm);
    assert(func_val.type == FUNC_TYPE);
    assert(!value_is_nil(&func_val));
    ((FUNC*)func_val.data.obj)->is_macro = 1;
    next(vm);
}

void macro_qm_impl(VM* vm) {
    VALUE func_val;
    VM_POP_ARG(&func_val, vm);
    assert(func_val.type == FUNC_TYPE);
    assert(!value_is_nil(&func_val));
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
    VM_POP_ARG(&func_name, vm);
    assert(func_name.type == SYMBOL_TYPE);
    VALUE func_val = lookup_by_symv(vm, &func_name);
    assert(func_val.type == FUNC_TYPE);
    compile_call(&vm->compiler, ((FUNC*)func_val.data.obj)->pnode);
    next(vm);
}

void begin_compilation_impl(VM* vm) {
    begin_compilation(&vm->compiler);
    next(vm);
}

void end_compilation_impl(VM* vm) {
    VALUE func_name;
    VM_POP_ARG(&func_name, vm);
    assert(func_name.type == SYMBOL_TYPE);
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
            assert(c != EOF);
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
    VM_POP_ARG(&fn, vm);
    assert(fn.type == STRING_TYPE);
    FILE* fp = fopen(((STRING*)fn.data.obj)->data, "r");
    FILE* tmp = vm->in;
    vm->in = fp;

    while(!feof(vm->in)) {
        pvm_eval(vm);
    }
    
    vm->in = tmp;
    fclose(fp);
    // This one doesn't need a next(). It happens in a different execution context.
}

void jump_macro_impl(VM* vm) {
    compile_jump(&vm->compiler);
    next(vm);
}

void cjump_macro_impl(VM* vm) {
    compile_cjump(&vm->compiler);
    next(vm);
}

void resolve_impl(VM* vm) {
    VALUE mark_id;
    VM_POP_ARG(&mark_id, vm);
    assert(mark_id.type == NUM_TYPE);
    compiler_resolve(&vm->compiler, mark_id.data.num);
    next(vm);
}

void drop_marks_impl(VM* vm) {
    VALUE mark_num;
    VM_POP_ARG(&mark_num, vm);
    assert(mark_num.type == NUM_TYPE);
    compiler_resolve(&vm->compiler, mark_num.data.num);
    next(vm);
}

#define PLIST_PROGRAM_SET(X) {.fp = &X ## _impl}

const PNODE primitives[] = {
    PRIMITIVE_FUNC_LIST(PLIST_PROGRAM_SET, PLIST_IGNORE, PLIST_PROGRAM_SET, PLIST_COMMA),
    PRIMITIVE_MACRO_LIST(PLIST_PROGRAM_SET, PLIST_IGNORE, PLIST_PROGRAM_SET, PLIST_COMMA)
};
