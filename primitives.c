#include "vm.h"
#include "primitives.h"
#include "object.h"
#include "func.h"
#include "symbol.h"
#include "compiler.h"
#include <stdlib.h>
#include <assert.h>
#include <math.h>

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

// ************
// * Literals *
// ************

void true_impl(VM* vm) {
    push(vm, PVM_TRUE);
    next(vm);
}

void false_impl(VM* vm) {
    push(vm, PVM_FALSE);
    next(vm);
}

void nil_impl(VM* vm) {
    push(vm, PVM_NIL);
    next(vm);
}

// **********************
// * Stack Manipulation *
// **********************

void push_impl(VM* vm) {
    push(vm, (++vm->curr)->value);
    next(vm);
}

void dup_impl(VM* vm) {
    VALUE x = pop(vm);
    push(vm, x);
    push(vm, x);
    next(vm);
}

void swap_impl(VM* vm) {
    VALUE a = pop(vm);
    VALUE b = pop(vm);
    push(vm, a);
    push(vm, b);
    next(vm);
}

void drop_impl(VM* vm) {
    pop(vm);
    next(vm);
}

// ********
// * Math *
// ********

void plus_impl(VM* vm) {
    VALUE a = pop(vm), b = pop(vm);
    assert(a.type == NUM_TYPE && b.type == NUM_TYPE);
    push(vm, num_value(a.data.num + b.data.num));
    next(vm);
}

void minus_impl(VM* vm) {
    VALUE a = pop(vm), b = pop(vm);
    assert(a.type == NUM_TYPE && b.type == NUM_TYPE);
    push(vm, num_value(b.data.num - a.data.num));
    next(vm);
}

void mul_impl(VM* vm) {
    VALUE a = pop(vm), b = pop(vm);
    assert(a.type == NUM_TYPE && b.type == NUM_TYPE);
    push(vm, num_value(b.data.num * a.data.num));
    next(vm);
}


void div_impl(VM* vm) {
    VALUE a = pop(vm), b = pop(vm);
    assert(a.type == NUM_TYPE && b.type == NUM_TYPE);
    push(vm, num_value(b.data.num / a.data.num));
    next(vm);
}

void mod_impl(VM* vm) {
    VALUE a = pop(vm), b = pop(vm);
    assert(a.type == NUM_TYPE && b.type == NUM_TYPE);
    push(vm, num_value(fmod(b.data.num, a.data.num)));
    next(vm);
}

// **************
// * Relational *
// **************

void gt_impl(VM* vm) {
    VALUE a = pop(vm), b = pop(vm);
    assert(a.type == NUM_TYPE && b.type == NUM_TYPE);
    push(vm, (VALUE){.type = BOOL_TYPE, .data.boolean = b.data.num > a.data.num});
    next(vm);
}

void lt_impl(VM* vm) {
    VALUE a = pop(vm), b = pop(vm);
    assert(a.type == NUM_TYPE && b.type == NUM_TYPE);
    push(vm, (VALUE){.type = BOOL_TYPE, .data.boolean = b.data.num < a.data.num});
    next(vm);
}

void gte_impl(VM* vm) {
    VALUE a = pop(vm), b = pop(vm);
    assert(a.type == NUM_TYPE && b.type == NUM_TYPE);
    push(vm, (VALUE){.type = BOOL_TYPE, .data.boolean = b.data.num >= a.data.num});
    next(vm);
}

void lte_impl(VM* vm) {
    VALUE a = pop(vm), b = pop(vm);
    assert(a.type == NUM_TYPE && b.type == NUM_TYPE);
    push(vm, (VALUE){.type = BOOL_TYPE, .data.boolean = b.data.num <= a.data.num});
    next(vm);
}

void eq_impl(VM* vm) {
    VALUE a = pop(vm);
    VALUE b = pop(vm);
    VALUE c = {.type = BOOL_TYPE, .data.boolean = values_equal(&a, &b)};
    push(vm, c);
    next(vm);
}

void not_eq_impl(VM* vm) {
    VALUE a = pop(vm);
    VALUE b = pop(vm);
    VALUE c = {.type = BOOL_TYPE, .data.boolean = !values_equal(&a, &b)};
    push(vm, c);
    next(vm);
}

// *************
// * Logic Ops *
// *************

void and_impl(VM* vm) {
    VALUE a = pop(vm), b = pop(vm);
    assert(a.type == BOOL_TYPE && b.type == BOOL_TYPE);
    push(vm, (VALUE){.type = BOOL_TYPE, .data.boolean = b.data.boolean && a.data.boolean});
    next(vm);
}

void or_impl(VM* vm) {
    VALUE a = pop(vm), b = pop(vm);
    assert(a.type == BOOL_TYPE && b.type == BOOL_TYPE);
    push(vm, (VALUE){.type = BOOL_TYPE, .data.boolean = b.data.boolean || a.data.boolean});
    next(vm);
}

void not_impl(VM* vm) {
    VALUE a = pop(vm);
    assert(a.type == BOOL_TYPE);
    push(vm, (VALUE){.type = BOOL_TYPE, .data.boolean = !a.data.boolean});
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
    VALUE c = pop(vm);
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
    VALUE v = pop(vm);
    assert(v.type == FUNC_TYPE);
    vm->instr = ((FUNC*)v.data.obj)->pnode->fp;
}

void dcall_impl(VM* vm) { // call a function object
    assert(vm->ret_sp <= &vm->ret_stack[RET_STACK_SIZE - 1]);
    *vm->ret_sp = vm->curr;
    ++vm->ret_sp;
    VALUE v = pop(vm);
    assert(v.type == FUNC_TYPE);
    vm->curr = ((FUNC*)v.data.obj)->pnode;
    next(vm);
}

// **************
// * Object Ops *
// **************

void dgetf_impl(VM* vm) {
    VALUE objval = pop(vm);
    VALUE key = pop(vm);
    assert(objval.type == OBJECT_TYPE);
    OBJECT* obj = (OBJECT*)objval.data.obj;
    VALUE val;
    map_get(&val, &obj->map, &key);
    push(vm, val);
    next(vm);
}

void meta_impl(VM* vm) {
    VALUE objval = pop(vm);
    assert(objval.type == OBJECT_TYPE || objval.type == FUNC_TYPE);
    assert(!value_is_nil(&objval));
    OBJECT* obj = (OBJECT*)objval.data.obj;
    objval.type = OBJECT_TYPE;
    objval.data.obj = (OBJECT_BASE*)obj->base.meta;
    push(vm, objval);
    next(vm);
}

// *********
// * Misc. *
// *********

void get_impl(VM* vm) {
    VALUE key = pop(vm);
    VALUE item;
    map_get(&item, &vm->global_scope->map, &key);
    push(vm, item);
    next(vm);
}

void set_impl(VM* vm) {
    VALUE item = pop(vm);
    VALUE key = pop(vm);
    map_put(&vm->global_scope->map, &key, &item);
    next(vm);
}

void setmac_impl(VM* vm) {
    VALUE func_val = pop(vm);
    assert(func_val.type == FUNC_TYPE);
    assert(!value_is_nil(&func_val));
    ((FUNC*)func_val.data.obj)->is_macro = 1;
    next(vm);
}

void macro_qm_impl(VM* vm) {
    VALUE func_val = pop(vm);
    assert(func_val.type == FUNC_TYPE);
    assert(!value_is_nil(&func_val));
    push(vm, (VALUE){.type = BOOL_TYPE, .data.boolean = ((FUNC*)func_val.data.obj)->is_macro});
    next(vm);
}

void type_impl(VM* vm) {
    VALUE v = pop(vm);
    switch(v.type) {
        case BOOL_TYPE:
            push(vm, symbol_value(vm, "boolean"));
            break;
        case NUM_TYPE:
            push(vm, symbol_value(vm, "number"));
            break;
        case FUNC_TYPE:
            push(vm, symbol_value(vm, "function"));
            break;
        case STRING_TYPE:
            push(vm, symbol_value(vm, "string"));
            break;
        case SYMBOL_TYPE:
            push(vm, symbol_value(vm, "symbol"));
            break;
        case OBJECT_TYPE:
            push(vm, symbol_value(vm, "object"));
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
    push(vm, v);
    next(vm);
}

void program_unread_impl(VM* vm) {
    VALUE v = pop(vm);
    program_unread(vm, &v);
    next(vm);
}

void compile_literal_impl(VM* vm) {
    VALUE v = pop(vm);
    compile_literal(&vm->compiler, v);
    next(vm);
}

void compile_call_impl(VM* vm) {
    VALUE func_name = pop(vm);
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
    VALUE func_name = pop(vm);
    assert(func_name.type == SYMBOL_TYPE);
    char const* name_str = ((SYMBOL*)func_name.data.obj)->name;
    VALUE func_val = func_value(end_compilation(&vm->compiler, name_str), vm->func_meta, name_str);
    map_put(&vm->global_scope->map, &func_name, &func_val);
    next(vm);
}

//
//void mark_impl(VM* vm) {
//    mark_helper(vm);
//    next(vm);
//}
//
//void resolve_impl(VM* vm) {
//    VALUE n = pop(vm);
//    assert(n.type == NUM_TYPE);
//    resolve_helper(vm, n.data.num);
//    next(vm);
//}
//
//void drop_marks_impl(VM* vm) {
//    VALUE n = pop(vm);
//    assert(n.type == NUM_TYPE);
//    drop_marks_helper(vm, n.data.num);
//    next(vm);
//}
//
//void start_defun_impl(VM* vm) {
//    //program_rewind(vm);
//    VALUE func_name = pop(vm);
//    assert(func_name.type == SYMBOL_TYPE);
//    PNODE* stub = fcons(vm, enter_impl);
//    FUNC* func = create_func(stub, vm->func_meta);
//    VALUE func_value = {.type = FUNC_TYPE, .data.obj = (OBJECT_BASE*)func};
//    map_put(&vm->global_scope->map, &func_name, &func_value);
//    set_debug_info(vm, stub, ((SYMBOL*)func_name.data.obj)->name);
//    //push(vm, func_value);
//    next(vm);
//}
//
//void end_defun_impl(VM* vm) {
//    PNODE* leave = ((FUNC*)lookup(vm, "leave").data.obj)->pnode; //fcons(vm, leave_impl);
//    ncons(vm, leave);
//    //program_flush(vm);
//    //fcons(vm, enter_impl);
//    next(vm);
//}
//
//void read_string_impl(VM* vm) {
//    char* data = NULL;
//    int len = 0;
//    
//    char buff[256] = {0};
//    int i, buff_used;
//    char c = 0;
//    
//    fgetc(vm->in);
//    do {
//        for(buff_used = 0; buff_used < 256 && c != '"'; ++buff_used) {
//            c = fgetc(vm->in);
//            assert(c != EOF);
//            buff[buff_used] = c;
//        }
//        
//        len += buff_used;
//        data = realloc(data, sizeof(char) * len);
//        memcpy(data + len - buff_used, buff, buff_used);
//    }while(buff[buff_used - 1] != '"');
//    
//    data[len - 1] = 0;
//    compile_literal_helper(vm, (VALUE){.type = STRING_TYPE, .data.obj = (OBJECT_BASE*)create_string(data, len - 1, NULL)});
//    next(vm);
//}
//
//void load_impl(VM* vm) {
//    /*
//    
//    PNODE* ps = vm->program_write_start;
//    
//    //printf("entered");
//    VALUE fn = pop(vm);
//    assert(fn.type == STRING_TYPE);
//    FILE* fp = fopen(((STRING*)fn.data.obj)->data, "r");
//    FILE* tmp = vm->in;
//    vm->in = fp;
//    PNODE* curr = vm->curr;
//    
//    printf("zomg! %x", vm->curr);
//    
//    eval_str(vm);
//    //assert(0);
//    printf("omg! %x %x %x %x", ps, vm->program_write_start, vm->program_write_pos, vm->curr);
//    vm->in = tmp;
//    fclose(fp);
//    
//    
//    //vm->curr = curr;
//    PNODE* exit = ((FUNC*)lookup(vm, "exit").data.obj)->pnode;
//    
//    
//    //fcons(vm, enter_impl);
//    PNODE* pn = ncons(vm, exit);
//    vm->curr = curr; //pn - 1;
//    printf("zomg! %x %x", vm->curr, pn);
//    
//    //vm->instr = vm->curr->fp;
//    //next(vm);*/
//}

#define PLIST_PROGRAM_SET(X) {.fp = &X ## _impl}

const PNODE primitives[] = {
    PRIMITIVE_FUNC_LIST(PLIST_PROGRAM_SET, PLIST_IGNORE, PLIST_PROGRAM_SET, PLIST_COMMA),
    PRIMITIVE_MACRO_LIST(PLIST_PROGRAM_SET, PLIST_IGNORE, PLIST_PROGRAM_SET, PLIST_COMMA)
};
