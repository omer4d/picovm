#include "vm.h"
#include "primitives.h"
#include "object.h"
#include "func.h"
#include <stdlib.h>
#include <assert.h>
#include <math.h>

void next(VM* vm) {
    ++vm->curr;
    vm->instr = vm->curr->into->fp;
}

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

void exit_impl(VM* vm) {
    vm->curr = NULL;
    vm->instr = NULL;
}

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

void pcall_impl(VM* vm) {
    assert(vm->ret_sp <= &vm->ret_stack[RET_STACK_SIZE - 1]);
    VALUE v = pop(vm);
    assert(v.type == FUNC_TYPE);
    vm->instr = ((FUNC*)v.data.obj)->pnode->fp;
}

void dcall_impl(VM* vm) {
    assert(vm->ret_sp <= &vm->ret_stack[RET_STACK_SIZE - 1]);
    *vm->ret_sp = vm->curr;
    ++vm->ret_sp;
    VALUE v = pop(vm);
    assert(v.type == FUNC_TYPE);
    vm->curr = ((FUNC*)v.data.obj)->pnode;
    next(vm);
}

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

void push_impl(VM* vm) {
    push(vm, (++vm->curr)->value);
    next(vm);
}

void eq_impl(VM* vm) {
    VALUE a = pop(vm);
    VALUE b = pop(vm);
    VALUE c = {.type = BOOL_TYPE, .data.boolean = values_equal(&a, &b) ? 1 : 0};
    push(vm, c);
    next(vm);
}
