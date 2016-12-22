#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <mem.h>

#include "macros.h"
#include "value.h"
#include "object.h"
#include "symbol.h"
#include "string.h"
#include "primitives.h"
#include "vm.h"
#include "func.h"
#include "tokenizer.h"
#include "compiler.h"

VM* create_vm() {
    VM* vm = malloc(sizeof(VM));
    
    vm->log_stream = fopen("log.txt", "w+");
    vm->in = stdin;
    
    vm->flags = 0;
    vm->read_queue_start = 0;
    vm->read_queue_end = 0;
    
    vm->xc.arg_stack = vm->arg_stack_data;
    vm->xc.ret_stack = vm->ret_stack_data;
    vm->xc.arg_sp = vm->xc.arg_stack;
    vm->xc.ret_sp = vm->xc.ret_stack;
    
    vm->xc.curr = NULL;
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
    
    init_compiler(&vm->compiler);
    
    return vm;
}

void destroy_vm(VM* vm) {
    fclose(vm->log_stream);
    
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

VALUE lookup_by_name(VM* vm, char const* name) {
    VALUE key = symbol_value(vm, name);
    VALUE item;
    map_get(&item, &vm->global_scope->map, &key);
    return item;
}

VALUE lookup_by_symv(VM* vm, VALUE const* key) {
    VALUE item;
    map_get(&item, &vm->global_scope->map, key);
    return item;
}

PNODE const* register_func(VM* vm, PNODE const* pnode, char const* name, int primitive) {
    VALUE key = symbol_value(vm, name);
    VALUE item = func_value(pnode, primitive ? vm->primitive_func_meta : vm->func_meta, name);
    map_put(&vm->global_scope->map, &key, &item);
    return pnode;
}

PNODE const* register_macro(VM* vm, PNODE const* pnode, char const* name, int primitive) {
    VALUE key = symbol_value(vm, name);
    VALUE item = func_value(pnode, primitive ? vm->primitive_func_meta : vm->func_meta, name);
    ((FUNC*)item.data.obj)->is_macro = 1;
    map_put(&vm->global_scope->map, &key, &item);
    return pnode;
}

void push(VM* vm, VALUE x) {
    ASSERT_PUSH(vm->xc.arg_stack, vm->xc.arg_sp, ARG_STACK_SIZE);
    *vm->xc.arg_sp = x;
    ++vm->xc.arg_sp;
}

VALUE pop(VM* vm) {
    ASSERT_POP(vm->xc.arg_stack, vm->xc.arg_sp);
    --vm->xc.arg_sp;
    return *vm->xc.arg_sp;
}

char const* lookup_debug_info(VM* vm, PNODE const* pnode) {
    if(pnode >= primitives && pnode < primitives + PRIMITIVE_NUM) {
        char const* public_name = primitive_names[pnode - primitives];
        return public_name[0] ? public_name : primitive_internal_names[pnode - primitives];
    }
    else {
        return find_compilation_context(&vm->compiler, pnode);
    }
}

void vm_log(VM* vm, char const *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
    va_start(ap, fmt);
    vfprintf(vm->log_stream, fmt, ap);
    va_end(ap);
}

void vm_signal_silent_error(VM* vm) {
    vm->instr = NULL;
    vm->flags |= compiler_is_compiling(&vm->compiler) ? PVM_COMPILE_TIME_ERROR : PVM_RUNTIME_ERROR;
}

void vm_signal_error(VM* vm, char const* msg, char const* primitive) {
    vm->instr = NULL;
    
    if(compiler_is_compiling(&vm->compiler)) {
        vm_log(vm, "Compile time error in '%s': %s\n", primitive, msg);
        vm->flags |= PVM_COMPILE_TIME_ERROR;
    }
    else {
        vm_log(vm, "Runtime error in '%s': %s\n", primitive, msg);
        vm->flags |= PVM_RUNTIME_ERROR;
    }
}

void vm_vsignal_error(VM* vm, char const *fmt0, ...) {
    char* prelude = compiler_is_compiling(&vm->compiler) ? "Compile time error: " : "Runtime error: ";
    char fmt[256];
    snprintf(fmt, 256, "%s %s", prelude, fmt0);
    
    vm->flags |= compiler_is_compiling(&vm->compiler) ? PVM_COMPILE_TIME_ERROR : PVM_RUNTIME_ERROR;
    
    va_list ap;
    va_start(ap, fmt0);
    vprintf(fmt, ap);
    va_end(ap);
    va_start(ap, fmt0);
    vfprintf(vm->log_stream, fmt, ap);
    va_end(ap);
}


void pvm_set_flags(VM* vm, int f) {
    vm->flags |= f;
}

void pvm_clear_flags(VM* vm, int f) {
    vm->flags &= ~f;
}


int pvm_test_flags(VM* vm, int f) {
    return vm->flags & f; 
}

void pvm_get_cc(VM_CONTINUATION_DATA* cc, VM* vm) {
    cc->curr = vm->xc.curr;
    cc->ret_stack = vm->xc.ret_stack;
    cc->ret_sp = vm->xc.ret_sp;
    memcpy(cc->ret_stack_data, vm->ret_stack_data, RET_STACK_SIZE);
}

void print_debug_row(VM* vm, VALUE* asp, PNODE const** rsp) {
    char tmp[256] = {};
    
    if(rsp && asp)
        vm_log(vm, "%-30s %-30s\n", value_to_string(tmp, asp), find_compilation_context(&vm->compiler, *rsp));
    else if(rsp)
        vm_log(vm, "%-30s %-30s\n", "", find_compilation_context(&vm->compiler, *rsp));
    else if(asp)
        vm_log(vm, "%-30s %-30s\n", value_to_string(tmp, asp), "");
}


void print_debug_info(VM* vm) {
    VALUE* asp;
    PNODE const** rsp;
    
    vm_log(vm, "%-30s %-30s\n", "ARG STACK", "CALL STACK");
    vm_log(vm, "_________________________________________\n");
    
    for(asp = vm->xc.arg_stack, rsp = vm->xc.ret_stack; asp < vm->xc.arg_sp || rsp < vm->xc.ret_sp; ++rsp, ++asp) {
        print_debug_row(vm, asp < vm->xc.arg_sp ? asp : NULL, rsp < vm->xc.ret_sp ? rsp : NULL);
    }
    
    rsp = &vm->xc.curr;
    print_debug_row(vm, asp < vm->xc.arg_sp ? asp : NULL, *rsp ? rsp : NULL);
    
    vm_log(vm, "\nAbout to execute: %s", vm->xc.curr ? lookup_debug_info(vm, vm->xc.curr->into) : "N/A");
    vm_log(vm, "\n\n\n\n");
    fflush(vm->log_stream);
}


//void print_debug_info(VM* vm) {
//    VALUE* asp;
//    PNODE const** rsp;
//    char tmp[256] = {};
//    
//    vm_log(vm, "%-30s %-30s\n", "ARG STACK", "CALL STACK");
//    vm_log(vm, "_________________________________________\n");
//    
//    for(asp = vm->xc.arg_stack, rsp = vm->xc.ret_stack; asp < vm->xc.arg_sp && rsp < vm->xc.ret_sp; ++rsp, ++asp) {
//        vm_log(vm, "%-30s %-30s\n", value_to_string(tmp, asp), find_compilation_context(&vm->compiler, *rsp));
//    }
//    
//    if(asp < vm->xc.arg_sp) {
//        rsp = &vm->xc.curr;
//        if(*rsp)
//            vm_log(vm, "%-30s %-30s\n", value_to_string(tmp, asp++), find_compilation_context(&vm->compiler, *rsp));
//        for(; asp < vm->xc.arg_sp; ++asp)
//            vm_log(vm, "%-30s %-30s\n", value_to_string(tmp, asp), "");
//    }
//    
//    /*
//    else if(rsp < vm->xc.ret_sp) {
//        vm_log(vm, "%-30s %-30s\n", "", find_compilation_context(&vm->compiler, vm->xc.curr->into));
//        for(; rsp < vm->xc.ret_sp; ++rsp)
//            vm_log(vm, "%-30s %-30s\n", "", find_compilation_context(&vm->compiler, *rsp));
//    }*/
//    
//    vm_log(vm, "\nAbout to execute: %s", vm->xc.curr ? lookup_debug_info(vm, vm->xc.curr->into) : "N/A");
//    vm_log(vm, "\n\n\n\n");
//    fflush(vm->log_stream);
//}

void pvm_loop(VM* vm) {
    //printf("Initial state: ");
    //print_debug_info(vm);
    
    while(vm->instr) {
        //print_debug_info(vm);
        //getch();
        vm->instr(vm);
    }
    
    //print_debug_info(vm);
}

VALUE parse_num(char const* str) {
    double d = strtod(str, NULL);
    VALUE v = {.type = NUM_TYPE, .data.num = d};
    return v;
}

VALUE parse_word(VM* vm, char const* str) {
    VALUE v = {.type = SYMBOL_TYPE, .data.obj = (OBJECT_BASE*)get_symbol(vm, str)};
    return v;
}

VALUE program_read(VM* vm) {
    char tok[256];
    TOK_TYPE tt;
    
    //vm_log(vm, "************ TRY TO READ *************\n");
    
    if(vm->read_queue_end > vm->read_queue_start)
        return vm->read_buff[(vm->read_queue_start++) % READ_BUFF_SIZE];
    else {
        do {
            tt = next_tok(tok, vm->in);
            //vm_log(vm, "************ READ %s (%d) *************\n", tok, tt);
            
            switch(tt) {
                case TOK_WORD:
                    return parse_word(vm, tok);
                case TOK_NUM:
                    return parse_num(tok);
                default:
                    break;
            }
        }while(!feof(vm->in));
        
        //vm_log(vm, "************ LAST READ %s (%d) *************\n", tok, tt);
        assert(!"unexpected eof");
    }
}

void program_unread(VM* vm, VALUE const* v) {
    vm->read_buff[(vm->read_queue_end++) % READ_BUFF_SIZE] = *v;
}

void callf(VM* vm, VALUE v) {
    PNODE const* run = ((FUNC*)lookup_by_name(vm, "run").data.obj)->pnode;
    push(vm, v);
    vm->xc.curr = run;
    next(vm);
    pvm_loop(vm);
}

VM_EXECUTION_CONTEXT pvm_protect_xc(VM* vm) {
    VM_EXECUTION_CONTEXT old_xc = vm->xc;
    vm->xc.arg_stack = vm->xc.arg_sp;
    vm->xc.ret_stack = vm->xc.ret_sp;
    return old_xc;
}

PNODE* pvm_compile(VM* vm) {
    char tok[256];
    TOK_TYPE tt;
    VALUE key, item;
    COMPILER* c = &vm->compiler;
    VM_EXECUTION_CONTEXT old_xc = pvm_protect_xc(vm);
    int old_ucc = unfinished_compilation_count(c);
    pvm_clear_flags(vm, PVM_COMPILE_TIME_ERROR);
    
    begin_compilation(c);
    for(tt = next_tok(tok, vm->in); tt != TOK_END && !pvm_test_flags(vm, PVM_COMPILE_TIME_ERROR); tt = next_tok(tok, vm->in)) {
        switch(tt) {
            case TOK_WORD:
                key = symbol_value(vm, tok);
                map_get(&item, &vm->global_scope->map, &key);
                
                if(value_is_nil(&item)) {
                    vm_vsignal_error(vm, "Undefined function '%s'.\n", tok);
                }else if(item.type != FUNC_TYPE) {
                    vm_vsignal_error(vm, "'%s' is not a function.\n", tok);
                }else {
                    FUNC* f = (FUNC*)item.data.obj;
                    
                    if(f->is_macro)
                        callf(vm, item);
                    else
                        compile_call(c, f->pnode);
                }
                break;
            case TOK_NUM:
                compile_literal(c, parse_num(tok));
                break;
            default:
                assert(!"Unhandled token type!");
                break;
        }
    }
    
    vm->xc = old_xc;
    
    if(pvm_test_flags(vm, PVM_COMPILE_TIME_ERROR)) {
        while(unfinished_compilation_count(c) != old_ucc) {
            drop_compilation(c);
        }
        return NULL;
    }else {
        compile_call(c, &primitives[exit_loc]);
        compile_call(c, &primitives[leave_loc]);
        return end_compilation(c, "eval");
    }
}

void pvm_run(VM* vm, PNODE* pn) {
    pvm_clear_flags(vm, PVM_RUNTIME_ERROR);
    vm->xc.ret_sp = vm->xc.ret_stack;
    vm->xc.curr = pn;
    next(vm);
    pvm_loop(vm);
}

void pvm_resume(VM* vm) {
    pvm_clear_flags(vm, PVM_USER_HALT);
    next(vm);
    pvm_loop(vm);
}

void pvm_continue(VM* vm, VM_CONTINUATION_DATA* cc) {
    vm->xc.curr = cc->curr;
    vm->xc.ret_stack = cc->ret_stack;
    vm->xc.ret_sp = cc->ret_sp;
    memcpy(vm->ret_stack_data, cc->ret_stack_data, RET_STACK_SIZE);
    pvm_loop(vm);
}