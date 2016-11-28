#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "value.h"
#include "types.h"

#define ARG_STACK_SIZE 1024
#define RET_STACK_SIZE 1024

typedef void (*CFUN)();

typedef union PNODE_t {
    union PNODE_t* into;
    CFUN fp;
}PNODE;

PNODE program[3*1000*1000];
PNODE* program_pos = program;

VALUE arg_stack[ARG_STACK_SIZE] = {0};
VALUE* arg_sp = arg_stack;

PNODE* ret_stack[RET_STACK_SIZE] = {0};
PNODE** ret_sp = ret_stack;

PNODE* curr;
void (*instr)();

PNODE* ncons(PNODE* n) {
    PNODE* node = program_pos;
    ++program_pos;
    node->into = n;
    return node;
}

PNODE* fcons(CFUN fp) {
    PNODE* node = program_pos;
    ++program_pos;
    node->fp = fp;
    return node;
}

void push(VALUE x) {
    assert(arg_sp <= &arg_stack[ARG_STACK_SIZE - 1]);
    *arg_sp = x;
    ++arg_sp;
}

VALUE pop() {
    assert(arg_sp > arg_stack);
    --arg_sp;
    return *arg_sp;
}

void next() {
    ++curr;
    instr = curr->into->fp;
}

void enter_impl() {
    assert(ret_sp <= &ret_stack[RET_STACK_SIZE - 1]);
    *ret_sp = curr;
    ++ret_sp;
    
    curr = curr->into;
	next();
}

void leave_impl() {
    assert(ret_sp > ret_stack);
    --ret_sp;
    curr = *ret_sp;
    next();
}

void dup_impl() {
    VALUE x = pop();
    push(x);
    push(x);
    next();
}

void plus_impl() {
    VALUE a = pop();
    VALUE b = pop();
    assert(a.type == NUM_TYPE && b.type == NUM_TYPE);
    push(num_value(a.data.num + b.data.num));
    next();
}

void exit_impl() {
    curr = NULL;
    instr = NULL;
}

void cjump_impl() {
    VALUE c = pop();
    assert(c.type == BOOL_TYPE);
    
	if(c.data.boolean) {
	   curr = curr->into;
	   instr = curr->into->fp;
	}
	
	else {
		++curr;
		next();
	}
}

PNODE* defun(int n, ...) {
    static PNODE* leave = NULL;
    
    if(!leave) {
        leave = fcons(leave_impl);
    }
    
    va_list argp;
    va_start(argp, n);
    
    PNODE* first = fcons(enter_impl);
    
    int i;
    for(i = 0; i < n; ++i) {
        ncons(va_arg(argp, PNODE*));
    }
    
    va_end(argp);
    ncons(leave);
    
    return first;
}

void print_arg_stack() {
    VALUE* sp;
    for(sp = arg_stack; sp != arg_sp; ++sp)
        printf("%f\n", sp->data.num);
}

void loop() {
    while(instr) {
        instr();
    }
}

typedef struct SYMBOL_t {
    OBJECT_BASE base;
    char const* name;
}SYMBOL;

OBJECT* sym_meta = NULL;

SYMBOL* create_symbol(char const* name) {
    static SYMBOL** sym_table = NULL;
    static int sym_table_cap = 0;
    static int sym_num = 0;
    
    if(!sym_table) {
        sym_table_cap = 8;
        sym_table = malloc(sizeof(SYMBOL*) * sym_table_cap);
    }
    
    if(!sym_meta) {
        sym_meta = create_object();
    }
    
    int i;
    for(i = 0; i < sym_num; ++i) {
        if(!strcmp(sym_table[i]->name, name))
            return sym_table[i];
    }
    
    if(sym_num >= sym_table_cap) {
        sym_table = realloc(sym_table, sym_table_cap * 2);
    }
    
    SYMBOL* sym = malloc(sizeof(SYMBOL));
    sym->base.meta = sym_meta;
    sym->name = name;
    sym_table[sym_num++] = sym;
    return sym;
}

typedef struct FUNC_t {
    OBJECT_BASE base;
    union PNODE_t* pnode;
}FUNC;

OBJECT* func_meta = NULL;

void dcall_impl() {
    assert(ret_sp <= &ret_stack[RET_STACK_SIZE - 1]);
    *ret_sp = curr;
    ++ret_sp;
    VALUE v = pop();
    assert(v.type == OBJECT_TYPE);
    assert(v.data.obj->meta == func_meta);
    curr = ((FUNC*)v.data.obj)->pnode;
    next();
}

void getf_impl() {
    assert(ret_sp <= &ret_stack[RET_STACK_SIZE - 2]);
    VALUE key = pop();
    VALUE obj = pop();
    
    
}

FUNC* create_func(PNODE* pnode) {
    FUNC* f = malloc(sizeof(FUNC));
    
    if(!func_meta) {
        func_meta = create_object();
        
        //VALUE call = create_func(dcall);
    }
    
    f->base.meta = func_meta;
    f->pnode = pnode;
    
    return f;
}

void destroy_func(FUNC* f) {
    free(f);
}

VALUE func_value(PNODE* pnode) {
    VALUE v;
    v.type = OBJECT_TYPE;
    v.data.obj = (OBJECT_BASE*)create_func(pnode);
    return v;
}

void init() {
    OBJECT* meta = create_object();
    meta->base.meta = meta;
    MAP* m = &meta->map;
    //map_put(m, 
}

int main() {
    init();
    
    PNODE* dup = fcons(dup_impl);
    PNODE* plus = fcons(plus_impl);
    PNODE* dcall = fcons(dcall_impl);
    PNODE* exit = fcons(exit_impl);
    
    PNODE* run = defun(2, dcall, exit);
    
    
    PNODE* dbl = defun(2, dup, plus);
    PNODE* quad = defun(2, dbl, dbl);
    PNODE* main = defun(2, dcall, plus);
    
    push(num_value(2));
    push(num_value(4));
    push(func_value(quad));
    push(func_value(main));
    
    curr = run;
    next();
    loop();

    print_arg_stack();
    
    getch();
    return 0;
}
