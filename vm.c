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

VM* create_vm() {
    VM* vm = malloc(sizeof(VM));
    int cell_num = 3 * 1000 * 1000;
    
    vm->arg_stack = calloc(ARG_STACK_SIZE, sizeof(VALUE));
    vm->arg_sp = vm->arg_stack;
    
    vm->ret_stack = calloc(RET_STACK_SIZE, sizeof(PNODE*));
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
    free(vm->arg_stack);
    free(vm->ret_stack);
    
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
    //vm->debug_info[node - vm->program] = debug;
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
            sprintf(str, "<function %s>", "unknown");
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
}

void loop(VM* vm) {
    //printf("Initial state: ");
    //print_debug_info(vm);
    
    while(vm->instr) {
        print_debug_info(vm);
        //getch();
        vm->instr(vm);
    }
    
    //print_debug_info();
}

void vm_run() {
    
}
