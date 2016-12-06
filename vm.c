#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "value.h"
#include "object.h"
#include "symbol.h"
#include "primitives.h"
#include "vm.h"
#include "tokenizer.h"
#include "func.h"

void set_debug_info(VM* vm, PNODE* node, char const* debug);
PNODE* register_func(VM* vm, PNODE* pnode, char const* name, int primitive);
PNODE* ncons(VM* vm, PNODE* n);
PNODE* fcons(VM* vm, CFUN fp);
PNODE* fcons_deb(VM* vm, CFUN fp, char const* name);
void mark(VM* vm);
void resolve(VM* vm, int n);
void drop_marks(VM* vm, int n);
VALUE lookup(VM* vm, char const* name);
PNODE* defun(VM* vm, int n, ...);
void set_method(VM* vm, OBJECT* object, char const* name, FUNC* func);

enum {
    PUSH, INTERNAL_FUNC_NUM
};

void init_private_funcs(VM* vm) {
    vm->program[PUSH].fp = push_impl;
    set_debug_info(vm, &vm->program[PUSH], "push");
    
    vm->program_pos = vm->program + INTERNAL_FUNC_NUM;
}

void compile_literal(VM* vm, VALUE val) {
    ncons(vm, &vm->program[PUSH]);
    PNODE* tmp = ncons(vm, NULL);
    tmp->value = val;
}

void init_global_scope(VM* vm) {
    PNODE* dup = register_func(vm, fcons(vm, dup_impl), "dup", 1);
    PNODE* swap = register_func(vm, fcons(vm, swap_impl), "swap", 1);
    PNODE* drop = register_func(vm, fcons(vm, drop_impl), "drop", 1);
    PNODE* plus = register_func(vm, fcons(vm, plus_impl), "+", 1);
    
    PNODE* meta = register_func(vm, fcons(vm, meta_impl), "meta", 1);
    PNODE* cjump = register_func(vm, fcons(vm, cjump_impl), "cjump", 1);
    PNODE* jump = register_func(vm, fcons(vm, jump_impl), "jump", 1);
    
    PNODE* exit = register_func(vm, fcons(vm, exit_impl), "exit", 1);
    
    PNODE* leave = register_func(vm, fcons(vm, leave_impl), "leave", 1);
    PNODE* fpush = &vm->program[PUSH];
    PNODE* eq = register_func(vm, fcons(vm, eq_impl), "eq", 1);
    
    PNODE* dbl = register_func(vm, defun(vm, 2, dup, plus), "dbl", 0);
    
    // **** methods ****
    
    PNODE* dcall = fcons_deb(vm, dcall_impl, "dcall");
    FUNC* dcall_func = create_func(dcall, vm->primitive_func_meta);
    
    PNODE* pcall = fcons_deb(vm, pcall_impl, "pcall");
    FUNC* pcall_func = create_func(pcall, vm->primitive_func_meta);
    
    PNODE* dgetf = fcons_deb(vm, dgetf_impl, "dgetf");
    FUNC* dgetf_func = create_func(dgetf, vm->primitive_func_meta);
    
    // **** call decl ****
    
    PNODE* call = fcons(vm, enter_impl);
    register_func(vm, call, "call", 0);
    ncons(vm, jump);
    PNODE* call_stub = ncons(vm, NULL);
    
    // **** getf ****
    
    PNODE* getf = fcons(vm, enter_impl);
    register_func(vm, getf, "getf", 0);

    ncons(vm, dup);
    ncons(vm, meta);
    
    ncons(vm, dup);
    ncons(vm, fpush);
    PNODE* tmp = ncons(vm, NULL);
    tmp->value.type = OBJECT_TYPE;
    tmp->value.data.obj = (OBJECT_BASE*)vm->default_meta;
    ncons(vm, eq);
    
    ncons(vm, cjump);
    mark(vm);
        ncons(vm, fpush);
        tmp = ncons(vm, NULL);
        tmp->value = symbol_value(vm, "index");
        ncons(vm, swap);
        ncons(vm, getf);
        ncons(vm, call);
        ncons(vm, jump);
    mark(vm);
    resolve(vm, 2);
        ncons(vm, drop);
        ncons(vm, dgetf);
    resolve(vm, 1);
    drop_marks(vm, 2);
    
    ncons(vm, leave);
    
    // **** call def ****
    
    PNODE* call_rest = ncons(vm, dup);
    call_stub->into = call_rest;
    
    ncons(vm, fpush);
    tmp = ncons(vm, NULL);
    tmp->value = (VALUE){.type = FUNC_TYPE, .data.obj = (OBJECT_BASE*)pcall_func}; //lookup(vm, "pcall");
    ncons(vm, eq);
    
    ncons(vm, cjump);
    mark(vm);
        ncons(vm, dup);
        ncons(vm, meta);
        
        ncons(vm, fpush);
        tmp = ncons(vm, NULL);
        tmp->value = symbol_value(vm, "call");
        ncons(vm, swap);
        ncons(vm, getf);
        ncons(vm, call);
        ncons(vm, jump);
    mark(vm);
    resolve(vm, 2);
        ncons(vm, pcall);
    resolve(vm, 1);
    
    ncons(vm, leave);
    
    // **** run ****
    
    register_func(vm, defun(vm, 2, call, exit), "run", 0);
    
    // **** init meta methods ****
    
    set_method(vm, vm->default_meta, "index", dgetf_func);
    set_method(vm, vm->func_meta, "call", dcall_func);
    set_method(vm, vm->primitive_func_meta, "call", pcall_func);
}

VM* create_vm() {
    VM* vm = malloc(sizeof(VM));
    init_tokenizer(&vm->tokenizer, "");
    int cell_num = 3 * 1000 * 1000;
    
    vm->program = calloc(cell_num, sizeof(PNODE));
    vm->debug_info = calloc(cell_num, sizeof(char*));
    vm->program_pos = vm->program;
    
    vm->arg_stack = calloc(ARG_STACK_SIZE, sizeof(VALUE));
    vm->arg_sp = vm->arg_stack;
    
    vm->ret_stack = calloc(RET_STACK_SIZE, sizeof(PNODE*));
    vm->ret_sp = vm->ret_stack;
    
    vm->mark_stack = calloc(MARK_STACK_SIZE, sizeof(PNODE));
    vm->mark_sp = vm->mark_stack;
    
    vm->curr = NULL;
    vm->instr = NULL;
    
    vm->default_meta = create_object(NULL);
    vm->default_meta->base.meta = vm->default_meta;
    vm->sym_meta = create_object(vm->default_meta);
    vm->func_meta = create_object(vm->default_meta);
    vm->primitive_func_meta = create_object(vm->default_meta);
    
    vm->sym_table_cap = 8;
    vm->sym_table = malloc(sizeof(SYMBOL*) * vm->sym_table_cap);
    vm->sym_num = 0;
    
    vm->global_scope = create_object(vm->default_meta);
    init_private_funcs(vm);
    init_global_scope(vm);
    
    return vm;
}

void destroy_vm(VM* vm) {
    free(vm->program);
    free(vm->debug_info);
    free(vm->arg_stack);
    free(vm->ret_stack);
    free(vm->mark_stack);
    
    destroy_object(vm->global_scope);
    
    free(vm->sym_table);
    
    free(vm);
}


void set_method(VM* vm, OBJECT* object, char const* name, FUNC* func) {
    VALUE key = symbol_value(vm, name);
    VALUE item = (VALUE){.type = FUNC_TYPE, .data.obj = (OBJECT_BASE*)func};
    map_put(&object->map, &key, &item);
}

void set_debug_info(VM* vm, PNODE* node, char const* debug) {
    vm->debug_info[node - vm->program] = debug;
}

VALUE lookup(VM* vm, char const* name) {
    VALUE key = symbol_value(vm, name);
    VALUE item;
    map_get(&item, &vm->global_scope->map, &key);
    return item;
}

PNODE* register_func(VM* vm, PNODE* pnode, char const* name, int primitive) {
    VALUE key = symbol_value(vm, name);
    VALUE item = func_value(pnode, primitive ? vm->primitive_func_meta : vm->func_meta);
    map_put(&vm->global_scope->map, &key, &item);
    set_debug_info(vm, pnode, name);
    return pnode;
}

PNODE* ncons(VM* vm, PNODE* n) {
    PNODE* node = vm->program_pos;
    ++vm->program_pos;
    node->into = n;
    return node;
}

PNODE* fcons(VM* vm, CFUN fp) {
    PNODE* node = vm->program_pos;
    ++vm->program_pos;
    node->fp = fp;
    return node;
}

PNODE* fcons_deb(VM* vm, CFUN fp, char const* name) {
    PNODE* pnode = fcons(vm, fp);
    set_debug_info(vm, pnode, name);
    return pnode;
}

void push(VM* vm, VALUE x) {
    assert(vm->arg_sp <= &vm->arg_stack[ARG_STACK_SIZE - 1]);
    *vm->arg_sp = x;
    ++vm->arg_sp;
}

VALUE pop(VM* vm) {
    assert(vm->arg_sp > vm->arg_stack);
    --vm->arg_sp;
    return *vm->arg_sp;
}



void mark(VM* vm) {
    assert(vm->mark_sp < vm->mark_stack + MARK_STACK_SIZE);
    PNODE* pn = ncons(vm, vm->mark_sp);
    vm->mark_sp->into = pn;
    ++vm->mark_sp;
}

void resolve(VM* vm, int n) {
    assert(vm->mark_sp - n >= vm->mark_stack);
    (vm->mark_sp - n)->into->into = vm->program_pos;
}

void drop_marks(VM* vm, int n) {
    vm->mark_sp -= n;
}

PNODE* defun(VM* vm, int n, ...) {
    static PNODE* leave = NULL;
    
    if(!leave) {
        leave = fcons(vm, leave_impl);
    }
    
    va_list argp;
    va_start(argp, n);
    
    PNODE* first = fcons(vm, enter_impl);
    
    int i;
    for(i = 0; i < n; ++i) {
        PNODE* pn = va_arg(argp, PNODE*);
        ncons(vm, pn);
    }
    
    va_end(argp);
    ncons(vm, leave);
    
    return first;
}

char const* lookup_debug_info(VM* vm, PNODE* pnode) {
    int idx = pnode - vm->program;
    return idx >= 0 ? (vm->debug_info[idx] ? vm->debug_info[idx] : "<no-info>") : "<invalid>";
}

char* value_to_string(char* str, VALUE* sp) {
    switch(sp->type) {
        case BOOL_TYPE:
            sprintf(str, "%s", sp->data.boolean ? "true" : "false");
            break;
        case NUM_TYPE:
            sprintf(str, "%f", sp->data.num);
            break;
        case FUNC_TYPE:
            sprintf(str, "<function %s>", "unknown");
            break;
        case STRING_TYPE:
            sprintf(str, "<string>");
            break;
        case SYMBOL_TYPE:
            sprintf(str, "'%s", ((SYMBOL*)sp->data.obj)->name);
            break;
        case OBJECT_TYPE:
            sprintf(str, value_is_nil(sp) ? "nil" : "<object>");
            break;
        default:
            assert(0);
    }
    
    return str;
}



void print_debug_info(VM* vm) {
    VALUE* asp;
    PNODE** rsp;
    char tmp[256] = {};
    
    printf("Next instruction: %s\n\n", vm->curr ? lookup_debug_info(vm, vm->curr->into) : "N/A");
    printf("%-30s %-30s\n", "ARG STACK", "CALL STACK");
    printf("_________________________________________\n");
    for(asp = vm->arg_stack, rsp = vm->ret_stack; asp < vm->arg_sp || rsp < vm->ret_sp; ++rsp, ++asp) {
        if(rsp < vm->ret_sp && asp < vm->arg_sp)
            printf("%-30s %-30s\n", value_to_string(tmp, asp), lookup_debug_info(vm, (*rsp)->into));
        else if(rsp < vm->ret_sp)
            printf("%-30s %-30s\n", "", lookup_debug_info(vm, (*rsp)->into));
        else
            printf("%-30s %-30s\n", value_to_string(tmp, asp), "");
    }
    
    printf("\n\n\n\n");
}

void loop(VM* vm) {
    while(vm->instr) {
        //print_debug_info();
        //getch();
        vm->instr(vm);
    }
    
    //print_debug_info();
}

VALUE parse_num(char const* str) {
    VALUE v = {.type = NUM_TYPE, .data.num = strtod(str, NULL)};
    return v;
}

VALUE parse_word(VM* vm, char const* str) {
    VALUE v = {.type = SYMBOL_TYPE, .data.obj = (OBJECT_BASE*)get_symbol(vm, str)};
    return v;
}

void eval_str(VM* vm, char const* str) {
    init_tokenizer(&vm->tokenizer, str);
    char tok[256];
    TOK_TYPE tt;
    VALUE key, item;
    PNODE* old_program_pos = vm->program_pos;
    PNODE* run = ((FUNC*)lookup(vm, "run").data.obj)->pnode;
    PNODE* func_start = fcons(vm, enter_impl);
    
    /*
    int i;
    for(i = 0; i < n; ++i) {
        PNODE* pn = va_arg(argp, PNODE*);
        ncons(vm, pn);
    }
    
    va_end(argp);
    ncons(vm, exit);*/
    
    
    for(tt = next_tok(&vm->tokenizer, tok); tt != TOK_END; tt = next_tok(&vm->tokenizer, tok)) {
        switch(tt) {
            case TOK_WORD:
                key = symbol_value(vm, tok);
                map_get(&item, &vm->global_scope->map, &key);
                
                if(value_is_nil(&item)) {
                    printf("Undefined function '%s'.\n", tok);
                }else if(item.type != FUNC_TYPE) {
                    printf("'%s' is not a function.\n", tok);
                }else {
                    FUNC* f = (FUNC*)item.data.obj;
                    if(f->is_macro) {
                        push(vm, item);
                        vm->curr = run;
                        next(vm);
                        loop(vm);
                    }
                    else {
                        ncons(vm, f->pnode);
                    }
                    //push(vm, item);
                    //vm->curr = run;
                    //next(vm);
                    //loop(vm);
                    
                }
                break;
            case TOK_NUM:
                //push(vm, parse_num(tok));
                compile_literal(vm, parse_num(tok));
                break;
        }
        
        
    }
    
    PNODE* exit = ((FUNC*)lookup(vm, "exit").data.obj)->pnode;
    ncons(vm, exit);
    vm->curr = func_start;
    next(vm);
    loop(vm);
    
    vm->program_pos = old_program_pos;
    print_debug_info(vm);
}
