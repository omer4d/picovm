#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

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
    
    vm->read_queue_start = 0;
    vm->read_queue_end = 0;
    
    vm->arg_sp = vm->arg_stack;
    vm->ret_sp = vm->ret_stack;
    
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

void set_debug_info(VM* vm, PNODE const* node, char const* debug) {
    //vm->debug_info[node - vm->program] = debug;
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
    set_debug_info(vm, pnode, name);
    return pnode;
}

PNODE const* register_macro(VM* vm, PNODE const* pnode, char const* name, int primitive) {
    VALUE key = symbol_value(vm, name);
    VALUE item = func_value(pnode, primitive ? vm->primitive_func_meta : vm->func_meta, name);
    ((FUNC*)item.data.obj)->is_macro = 1;
    map_put(&vm->global_scope->map, &key, &item);
    set_debug_info(vm, pnode, name);
    return pnode;
}

void push(VM* vm, VALUE x) {
    ASSERT_PUSH(vm->arg_stack, vm->arg_sp, ARG_STACK_SIZE);
    *vm->arg_sp = x;
    ++vm->arg_sp;
}

VALUE pop(VM* vm) {
    ASSERT_POP(vm->arg_stack, vm->arg_sp);
    --vm->arg_sp;
    return *vm->arg_sp;
}

/*
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
}*/

char* value_to_string(char* str, VALUE* sp) {
    switch(sp->type) {
        case BOOL_TYPE:
            sprintf(str, "%s", sp->data.boolean ? "true" : "false");
            break;
        case NUM_TYPE:
            sprintf(str, "%f", sp->data.num);
            break;
        case FUNC_TYPE:
            sprintf(str, "<function %s>", ((FUNC*)sp->data.obj)->name ? ((FUNC*)sp->data.obj)->name : "unknown");
            break;
        case STRING_TYPE:
            sprintf(str, "\"%s\"", ((STRING*)sp->data.obj)->data);
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

void vm_signal_error(VM* vm, char const* msg, char const* primitive) {
    vm->instr = NULL;
    vm_log(vm, "Error in '%s': %s\n", primitive, msg);
}

void print_debug_info(VM* vm) {
    VALUE* asp;
    PNODE const** rsp;
    char tmp[256] = {};
    
    vm_log(vm, "%-30s %-30s\n", "ARG STACK", "CALL STACK");
    vm_log(vm, "_________________________________________\n");
    for(asp = vm->arg_stack, rsp = vm->ret_stack; asp < vm->arg_sp || rsp < vm->ret_sp; ++rsp, ++asp) {
        if(rsp < vm->ret_sp && asp < vm->arg_sp)
            vm_log(vm, "%-30s %-30s\n", value_to_string(tmp, asp), find_compilation_context(&vm->compiler, (*rsp)->into));
        else if(rsp < vm->ret_sp)
            vm_log(vm, "%-30s %-30s\n", "", find_compilation_context(&vm->compiler, (*rsp)->into));
        else
            vm_log(vm, "%-30s %-30s\n", value_to_string(tmp, asp), "");
    }
    vm_log(vm, "\nAbout to execute: %s", vm->curr ? lookup_debug_info(vm, vm->curr->into) : "N/A");
    
    vm_log(vm, "\n\n\n\n");
    fflush(vm->log_stream);
}


/*
void print_debug_info(VM* vm) {
    VALUE* asp;
    PNODE const** rsp;
    char tmp[256] = {};
    
    printf("Next instruction: %s\n\n", vm->curr ? "unknown" : "N/A");
    printf("%-30s %-30s\n", "ARG STACK", "CALL STACK");
    printf("_________________________________________\n");
    for(asp = vm->arg_stack, rsp = vm->ret_stack; asp < vm->arg_sp || rsp < vm->ret_sp; ++rsp, ++asp) {
        if(rsp < vm->ret_sp && asp < vm->arg_sp)
            printf("%-30s %-30s\n", value_to_string(tmp, asp), "unknown");
        else if(rsp < vm->ret_sp)
            printf("%-30s %-30s\n", "", "unknown");
        else
            printf("%-30s %-30s\n", value_to_string(tmp, asp), "");
    }
    
    printf("\n\n\n\n");
}*/

void loop(VM* vm) {
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

void pvm_eval(VM* vm) {
    char tok[256];
    TOK_TYPE tt;
    VALUE key, item;
    COMPILER* c = &vm->compiler;
    PNODE const* run = ((FUNC*)lookup_by_name(vm, "run").data.obj)->pnode;
    
    begin_compilation(c);
    for(tt = next_tok(tok, vm->in); tt != TOK_END; tt = next_tok(tok, vm->in)) {
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
                        compile_call(c, f->pnode);
                    }
                }
                break;
            case TOK_NUM:
                compile_literal(c, parse_num(tok));
                break;
            default:
                assert(0);
                break;
        }
    }
    
    compile_call(c, &primitives[exit_loc]);
    compile_call(c, &primitives[leave_loc]);
    PNODE* f = end_compilation(c, "eval");
    
    vm->curr = f;
    next(vm);
    loop(vm);
    
    free(f);
    print_debug_info(vm);
}
