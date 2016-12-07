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

char const* lookup_debug_info(VM* vm, PNODE* pnode);
void print_debug_info(VM* vm);
char* value_to_string(char* str, VALUE* sp);
void set_debug_info(VM* vm, PNODE* node, char const* debug);
PNODE* register_func(VM* vm, PNODE* pnode, char const* name, int primitive);
PNODE* register_macro(VM* vm, PNODE* pnode, char const* name, int primitive);
PNODE* ncons(VM* vm, PNODE* n);
PNODE* fcons(VM* vm, CFUN fp);
PNODE* fcons_deb(VM* vm, CFUN fp, char const* name);
void push(VM* vm, VALUE v);
void mark_helper(VM* vm);
void resolve_helper(VM* vm, int n);
void drop_marks_helper(VM* vm, int n);
VALUE lookup(VM* vm, char const* name);
PNODE* defun(VM* vm, int n, ...);
void set_method(VM* vm, OBJECT* object, char const* name, FUNC* func);

enum {
    PUSH, INTERNAL_FUNC_NUM
};

void program_flush(VM* vm) {
    vm->program_write_start = vm->program_write_pos;
}

void program_rewind(VM* vm) {
    vm->program_write_pos = vm->program_write_start;
}

void init_private_funcs(VM* vm) {
    vm->program[PUSH].fp = push_impl;
    set_debug_info(vm, &vm->program[PUSH], "push");
    
    vm->program_write_pos = vm->program + INTERNAL_FUNC_NUM;
    program_flush(vm);
}

VALUE parse_num(char const* str) {
    VALUE v = {.type = NUM_TYPE, .data.num = strtod(str, NULL)};
    return v;
}

VALUE parse_word(VM* vm, char const* str) {
    VALUE v = {.type = SYMBOL_TYPE, .data.obj = (OBJECT_BASE*)get_symbol(vm, str)};
    return v;
}

VALUE program_read(VM* vm) {
    char tok[256];
    TOK_TYPE tt = next_tok(&vm->tokenizer, tok);
    switch(tt) {
        case TOK_WORD:
            return parse_word(vm, tok);
        case TOK_NUM:
            return parse_num(tok);
        default:
            assert(0);
    }
}

void compile_literal_helper(VM* vm, VALUE val) {
    ncons(vm, &vm->program[PUSH]);
    PNODE* tmp = ncons(vm, NULL);
    tmp->value = val;
}

void program_read_impl(VM* vm) {
    VALUE v = program_read(vm);
    push(vm, v);
    next(vm);
}

void compile_literal_impl(VM* vm) {
    VALUE v = pop(vm);
    compile_literal_helper(vm, v);
    next(vm);
}

void compile_call_impl(VM* vm) {
    VALUE func_name = pop(vm), func_val;
    FUNC* func;
    assert(func_name.type == SYMBOL_TYPE);
    
    map_get(&func_val, &vm->global_scope->map, &func_name);
    
    assert(!value_is_nil(&func_val));
    
    ncons(vm, ((FUNC*)func_val.data.obj)->pnode);
    next(vm);   
}

void mark_impl(VM* vm) {
    mark_helper(vm);
    next(vm);
}

void resolve_impl(VM* vm) {
    VALUE n = pop(vm);
    assert(n.type == NUM_TYPE);
    resolve_helper(vm, n.data.num);
    next(vm);
}

void drop_marks_impl(VM* vm) {
    VALUE n = pop(vm);
    assert(n.type == NUM_TYPE);
    drop_marks_helper(vm, n.data.num);
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

void compile_impl(VM* vm) {
    VALUE v = pop(vm);
    VALUE item;
    FUNC* f;
    
    switch(v.type) {
        case SYMBOL_TYPE:
            map_get(&item, &vm->global_scope->map, &v);
            f = (FUNC*)item.data.obj;
            if(f->is_macro) {
                //assert(vm->ret_sp <= &vm->ret_stack[RET_STACK_SIZE - 1]);
                //*vm->ret_sp = vm->curr;
                //++vm->ret_sp;
                
                vm->instr = (f->pnode + 1)->fp;
                printf("zomg! %s, %x %x", lookup_debug_info(vm, f->pnode), f->pnode->fp, enter_impl);
            }
            else {
                ncons(vm, f->pnode);
                next(vm);
            }   
            break;
        default:
            compile_literal_helper(vm, v);
            next(vm);
            break;
    }
}

void macro_qm_impl(VM* vm) {
    //VALUE func_name = pop(vm);
    VALUE func_val = pop(vm);
    //assert(func_name.type == SYMBOL_TYPE);
    //map_get(&func_val, &vm->global_scope->map, &func_name);
    assert(func_val.type == FUNC_TYPE);
    assert(!value_is_nil(&func_val));
    push(vm, (VALUE){.type = BOOL_TYPE, .data.boolean = ((FUNC*)func_val.data.obj)->is_macro});
    next(vm);
}

void start_defun_impl(VM* vm) {
    program_rewind(vm);
    VALUE func_name = pop(vm);
    assert(func_name.type == SYMBOL_TYPE);
    PNODE* stub = fcons(vm, enter_impl);
    FUNC* func = create_func(stub, vm->func_meta);
    VALUE func_value = {.type = FUNC_TYPE, .data.obj = (OBJECT_BASE*)func};
    map_put(&vm->global_scope->map, &func_name, &func_value);
    set_debug_info(vm, stub, ((SYMBOL*)func_name.data.obj)->name);
    //push(vm, func_value);
    next(vm);
}

void end_defun_impl(VM* vm) {
    PNODE* leave = ((FUNC*)lookup(vm, "leave").data.obj)->pnode; //fcons(vm, leave_impl);
    ncons(vm, leave);
    program_flush(vm);
    fcons(vm, enter_impl);
    next(vm);
}

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

void init_global_scope(VM* vm) {
    PNODE* dup = register_func(vm, fcons(vm, dup_impl), "dup", 1);
    PNODE* swap = register_func(vm, fcons(vm, swap_impl), "swap", 1);
    PNODE* drop = register_func(vm, fcons(vm, drop_impl), "drop", 1);
    PNODE* plus = register_func(vm, fcons(vm, plus_impl), "+", 1);
    
    PNODE* get = register_func(vm, fcons(vm, get_impl), "get", 1);
    PNODE* set = register_func(vm, fcons(vm, set_impl), "set", 1);
    
    PNODE* meta = register_func(vm, fcons(vm, meta_impl), "meta", 1);
    PNODE* cjump = register_func(vm, fcons(vm, cjump_impl), "cjump", 1);
    PNODE* jump = register_func(vm, fcons(vm, jump_impl), "jump", 1);
    
    PNODE* exit = register_func(vm, fcons(vm, exit_impl), "exit", 1);
    
    PNODE* leave = register_func(vm, fcons(vm, leave_impl), "leave", 1);
    PNODE* fpush = &vm->program[PUSH];
    PNODE* eq = register_func(vm, fcons(vm, eq_impl), "eq", 1);
    
    PNODE* macro_qm = register_func(vm, fcons(vm, macro_qm_impl), "macro?", 1);
    PNODE* setmac = register_func(vm, fcons(vm, setmac_impl), "setmac", 1);
    PNODE* program_read = register_macro(vm, fcons(vm, program_read_impl), ">>", 1);
    PNODE* compile_literal = register_macro(vm, fcons(vm, compile_literal_impl), "compile-literal", 1);
    PNODE* compile_call = register_macro(vm, fcons(vm, compile_call_impl), "compile-call", 1);

    PNODE* mark = register_macro(vm, fcons(vm, mark_impl), "mark", 1);
    PNODE* resolve = register_macro(vm, fcons(vm, resolve_impl), "resolve", 1);
    PNODE* drop_marks = register_macro(vm, fcons(vm, drop_marks_impl), "drop-marks", 1);

    PNODE* quote = register_macro(vm, defun(vm, 2, program_read, compile_literal), "'", 0);
    PNODE* type = register_func(vm, fcons(vm, type_impl), "type", 0);
    
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
    mark_helper(vm);
        ncons(vm, fpush);
        tmp = ncons(vm, NULL);
        tmp->value = symbol_value(vm, "index");
        ncons(vm, swap);
        ncons(vm, getf);
        ncons(vm, call);
        ncons(vm, jump);
    mark_helper(vm);
    resolve_helper(vm, 2);
        ncons(vm, drop);
        ncons(vm, dgetf);
    resolve_helper(vm, 1);
    drop_marks_helper(vm, 2);
    
    ncons(vm, leave);
    
    // **** call def ****
    
    PNODE* call_rest = ncons(vm, dup);
    call_stub->into = call_rest;
    
    ncons(vm, fpush);
    tmp = ncons(vm, NULL);
    tmp->value = (VALUE){.type = FUNC_TYPE, .data.obj = (OBJECT_BASE*)pcall_func}; //lookup(vm, "pcall");
    ncons(vm, eq);
    
    ncons(vm, cjump);
    mark_helper(vm);
        ncons(vm, dup);
        ncons(vm, meta);
        
        ncons(vm, fpush);
        tmp = ncons(vm, NULL);
        tmp->value = symbol_value(vm, "call");
        ncons(vm, swap);
        ncons(vm, getf);
        ncons(vm, call);
        ncons(vm, jump);
    mark_helper(vm);
    resolve_helper(vm, 2);
        ncons(vm, pcall);
    resolve_helper(vm, 1);
    drop_marks_helper(vm, 2);
    
    ncons(vm, leave);
    
    // **** compile ****
    
    PNODE* compile = fcons(vm, enter_impl);
    register_macro(vm, compile, "compile", 1);
    
    ncons(vm, dup);
    ncons(vm, type);
    compile_literal_helper(vm, symbol_value(vm, "symbol"));
    ncons(vm, eq);
    ncons(vm, cjump); mark_helper(vm);
        //not symbol
        ncons(vm, compile_literal);
        ncons(vm, jump); mark_helper(vm);
    
        //is symbol
        resolve_helper(vm, 2);
        ncons(vm, dup);
        ncons(vm, get);
        ncons(vm, macro_qm);
        ncons(vm, cjump); mark_helper(vm);
            // not macro
            ncons(vm, compile_call);
            ncons(vm, jump); mark_helper(vm);
    
            // is macro
            resolve_helper(vm, 2);
            ncons(vm, get);
            ncons(vm, call);
    
    resolve_helper(vm, 1);
    drop_marks_helper(vm, 2);
    
    resolve_helper(vm, 1);
    drop_marks_helper(vm, 2);
    ncons(vm, leave);
    
    // **** defun ****
    
    PNODE* start_defun = fcons(vm, start_defun_impl);
    set_debug_info(vm, start_defun, "start_defun");
    
    PNODE* end_defun = fcons(vm, end_defun_impl);
    set_debug_info(vm, end_defun, "end_defun");
    
    PNODE* loop;
    PNODE* defn = fcons(vm, enter_impl);
    register_macro(vm, defn, "defun", 0);
    
    ncons(vm, program_read);
    ncons(vm, start_defun);
    loop = ncons(vm, program_read);
    ncons(vm, dup);
    compile_literal_helper(vm, symbol_value(vm, "end"));
    ncons(vm, eq);
    ncons(vm, cjump);
    mark_helper(vm);
    // if false
        ncons(vm, compile);
        ncons(vm, jump);
        ncons(vm, loop);
    // if true
        resolve_helper(vm, 1);
        ncons(vm, drop);
        ncons(vm, end_defun);
        ncons(vm, leave);
    drop_marks_helper(vm, 1);
    
    // **** run ****
    
    register_func(vm, defun(vm, 2, call, exit), "run", 0);
    
    // **** init meta methods ****
    
    set_method(vm, vm->default_meta, "index", dgetf_func);
    set_method(vm, vm->func_meta, "call", dcall_func);
    set_method(vm, vm->primitive_func_meta, "call", pcall_func);
    
    program_flush(vm);
}

VM* create_vm() {
    VM* vm = malloc(sizeof(VM));
    init_tokenizer(&vm->tokenizer, "");
    int cell_num = 3 * 1000 * 1000;
    
    vm->program = calloc(cell_num, sizeof(PNODE));
    vm->debug_info = calloc(cell_num, sizeof(char*));
    vm->program_write_start = vm->program;
    vm->program_write_pos = vm->program;
    
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
    
    int i;
    for(i = 0; i < vm->sym_num; ++i)
        destroy_symbol(vm->sym_table[i]);
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

PNODE* register_macro(VM* vm, PNODE* pnode, char const* name, int primitive) {
    VALUE key = symbol_value(vm, name);
    VALUE item = func_value(pnode, primitive ? vm->primitive_func_meta : vm->func_meta);
    ((FUNC*)item.data.obj)->is_macro = 1;
    map_put(&vm->global_scope->map, &key, &item);
    set_debug_info(vm, pnode, name);
    return pnode;
}

PNODE* ncons(VM* vm, PNODE* n) {
    PNODE* node = vm->program_write_pos;
    ++vm->program_write_pos;
    node->into = n;
    return node;
}

PNODE* fcons(VM* vm, CFUN fp) {
    PNODE* node = vm->program_write_pos;
    ++vm->program_write_pos;
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



void mark_helper(VM* vm) {
    assert(vm->mark_sp < vm->mark_stack + MARK_STACK_SIZE);
    PNODE* pn = ncons(vm, vm->mark_sp);
    vm->mark_sp->into = pn;
    ++vm->mark_sp;
}

void resolve_helper(VM* vm, int n) {
    assert(vm->mark_sp - n >= vm->mark_stack);
    (vm->mark_sp - n)->into->into = vm->program_write_pos;
}

void drop_marks_helper(VM* vm, int n) {
    assert(vm->mark_sp - n >= vm->mark_stack);
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
        //print_debug_info(vm);
        //getch();
        vm->instr(vm);
    }
    
    //print_debug_info();
}

void eval_str(VM* vm, char const* str) {
    init_tokenizer(&vm->tokenizer, str);
    char tok[256];
    TOK_TYPE tt;
    VALUE key, item;
    //PNODE* old_program_pos = vm->program_write_pos;
    PNODE* run = ((FUNC*)lookup(vm, "run").data.obj)->pnode;
    /*PNODE* func_start = */fcons(vm, enter_impl);
    
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
                    /*
                    push(vm, item);
                    vm->curr = run;
                    next(vm);
                    loop(vm);*/
                    
                }
                break;
            case TOK_NUM:
                //push(vm, parse_num(tok));
                compile_literal_helper(vm, parse_num(tok));
                break;
        }
        
        
    }
    
    //printf("end!");
    PNODE* exit = ((FUNC*)lookup(vm, "exit").data.obj)->pnode;
    ncons(vm, exit);
    vm->curr = vm->program_write_start;//old_program_pos;//func_start;
    next(vm);
    loop(vm);
    program_rewind(vm);
    print_debug_info(vm);
}

