#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>

#include "value.h"
#include "types.h"
#include "symbol.h"

#define ARG_STACK_SIZE 1024
#define RET_STACK_SIZE 1024
#define MARK_STACK_SIZE 256

struct VM_t;

typedef void (*CFUN)(struct VM_t*);

typedef union PNODE_t {
    union PNODE_t* into;
    CFUN fp;
    VALUE value;
}PNODE;

typedef struct VM_t {
    PNODE* program;
    char const** debug_info;
    PNODE* program_pos;

    VALUE* arg_stack;
    VALUE* arg_sp;

    PNODE** ret_stack;
    PNODE** ret_sp;

    PNODE* mark_stack;
    PNODE* mark_sp;

    PNODE* curr;
    CFUN instr;
}VM;

VM* create_vm() {
    VM* vm = malloc(sizeof(VM));
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
    
    return vm;
}



void set_debug_info(VM* vm, PNODE* node, char const* debug) {
    vm->debug_info[node - vm->program] = debug;
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
    VALUE a = pop(vm);
    VALUE b = pop(vm);
    assert(a.type == NUM_TYPE && b.type == NUM_TYPE);
    push(vm, num_value(a.data.num + b.data.num));
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

void mark(VM* vm) {
    assert(vm->mark_sp < vm->mark_stack + MARK_STACK_SIZE);
    PNODE* pn = ncons(vm, vm->mark_sp);
    vm->mark_sp->into = pn;
    ++vm->mark_sp;
}

void resolve(VM* vm, int n) {
    assert(vm->mark_sp - n >= vm->mark_stack);
    //--vm->mark_sp;
    (vm->mark_sp - n)->into->into = vm->program_pos;
}

void drop_marks(VM* vm, int n) {
    vm->mark_sp -= n;
}

PNODE* mark_placeholder;
PNODE* resolve_placeholder;

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
        //if(pn == mark_placeholder)
        //    mark();
        //else if(pn == resolve_placeholder)
        //    resolve();
        //else
            ncons(vm, pn);
    }
    
    va_end(argp);
    ncons(vm, leave);
    
    return first;
}



typedef struct FUNC_t {
    OBJECT_BASE base;
    union PNODE_t* pnode;
}FUNC;

void dcall_impl(VM* vm) {
    assert(vm->ret_sp <= &vm->ret_stack[RET_STACK_SIZE - 1]);
    *vm->ret_sp = vm->curr;
    ++vm->ret_sp;
    VALUE v = pop(vm);
    assert(v.type == FUNC_TYPE);
    vm->curr = ((FUNC*)v.data.obj)->pnode;
    next(vm);
}

char const* lookup_debug_info(VM* vm, PNODE*);

void pcall_impl(VM* vm) {
    assert(vm->ret_sp <= &vm->ret_stack[RET_STACK_SIZE - 1]);
    VALUE v = pop(vm);
    assert(v.type == FUNC_TYPE);
    vm->instr = ((FUNC*)v.data.obj)->pnode->fp;
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

PNODE* pcall;
PNODE* dcall;
VALUE pcall_val;
VALUE dcall_val;

VALUE func_value(VM* vm, PNODE* pnode, int primitive);
PNODE* register_func(VM* vm, PNODE*, char const*, int primitive);
OBJECT* func_meta = NULL;
OBJECT* primitive_func_meta = NULL;

FUNC* create_func(VM* vm, PNODE* pnode, int primitive) {
    FUNC* f = malloc(sizeof(FUNC));
    
    if(!func_meta) {
        func_meta = create_object();
        primitive_func_meta = create_object();
        
        dcall = register_func(vm, fcons(vm, dcall_impl), "dcall", 1);
        pcall = register_func(vm, fcons(vm, pcall_impl), "pcall", 1);
        

        pcall_val = func_value(vm, pcall, 1);
        VALUE key = symbol_value("call");
        VALUE item = pcall_val;
        map_put(&primitive_func_meta->map, &key, &item);
        
        
        dcall_val = func_value(vm, dcall, 1);
        key = symbol_value("call");
        item = dcall_val;
        map_put(&func_meta->map, &key, &item);
        
        
        
        
        //key = symbol_value("index");
        
        
        //VALUE call = create_func(dcall);
    }
    
    f->base.meta = primitive ? primitive_func_meta : func_meta;
    f->pnode = pnode;
    
    return f;
}

void destroy_func(FUNC* f) {
    free(f);
}

VALUE func_value(VM* vm, PNODE* pnode, int primitive) {
    VALUE v;
    v.type = FUNC_TYPE;
    v.data.obj = (OBJECT_BASE*)create_func(vm, pnode, primitive);
    return v;
}











OBJECT* global_scope;

PNODE* register_func(VM* vm, PNODE* pnode, char const* name, int primitive) {
    VALUE key = symbol_value(name);
    VALUE item = func_value(vm, pnode, primitive);
    map_put(&global_scope->map, &key, &item);
    set_debug_info(vm, pnode, name);
    return pnode;
}

PNODE* run = NULL;

void set_method(VM* vm, OBJECT* object, char const* name, PNODE* func, int primitive) {
    VALUE key = symbol_value(name);
    VALUE item = func_value(vm, func, primitive);
    map_put(&object->map, &key, &item);
}



void init(VM* vm) {
    mark_placeholder = malloc(sizeof(PNODE));
    resolve_placeholder = malloc(sizeof(PNODE));
    
    OBJECT* metaobj = create_object();
    metaobj->base.meta = metaobj;
    init_object_system(metaobj);
    
    global_scope = create_object();
    
    //PNODE* dcall = register_func(vm, fcons(vm, dcall_impl), "dcall", 1);
    //PNODE* exit = register_func(vm, fcons(vm, exit_impl), "exit", 1);
    
    
    
    PNODE* dup = register_func(vm, fcons(vm, dup_impl), "dup", 1);
    PNODE* swap = register_func(vm, fcons(vm, swap_impl), "swap", 1);
    PNODE* drop = register_func(vm, fcons(vm, drop_impl), "drop", 1);
    PNODE* plus = register_func(vm, fcons(vm, plus_impl), "+", 1);
    PNODE* dcall = register_func(vm, fcons(vm, dcall_impl), "dcall", 1);
   
    
    PNODE* meta = register_func(vm, fcons(vm, meta_impl), "meta", 1);
    PNODE* dgetf = register_func(vm, fcons(vm, dgetf_impl), "dgetf", 1);
    
    PNODE* cjump = register_func(vm, fcons(vm, cjump_impl), "cjump", 1);
    PNODE* jump = register_func(vm, fcons(vm, jump_impl), "jump", 1);
    
    PNODE* exit = register_func(vm, fcons(vm, exit_impl), "exit", 1);
    
    PNODE* leave = register_func(vm, fcons(vm, leave_impl), "leave", 1);
    PNODE* fpush = register_func(vm, fcons(vm, push_impl), "push", 1);
    PNODE* eq = register_func(vm, fcons(vm, eq_impl), "eq", 1);
    
    
    PNODE* dbl = register_func(vm, defun(vm, 2, dup, plus), "dbl", 0);

    
    
    set_method(vm, metaobj, "index", dgetf, 1);
    
    
    PNODE* call = fcons(vm, enter_impl);
    register_func(vm, call, "call", 0);
    ncons(vm, jump);
    PNODE* call_stub = ncons(vm, NULL);
    
    // ********
    // * getf *
    // ********
    PNODE* getf = fcons(vm, enter_impl);
    register_func(vm, getf, "getf", 0);

    ncons(vm, dup);
    ncons(vm, meta);
    
    ncons(vm, dup);
    ncons(vm, fpush);
    PNODE* tmp = ncons(vm, NULL);
    tmp->value.type = OBJECT_TYPE;
    tmp->value.data.obj = (OBJECT_BASE*)default_meta;
    ncons(vm, eq);
    
    ncons(vm, cjump);
    mark(vm);
        ncons(vm, fpush);
        tmp = ncons(vm, NULL);
        tmp->value = symbol_value("index");
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
    
    
    // ********
    // * call *
    // ********
    
    /*
    dup [ pcall ] eq
    cjump mark
        dup meta [ call ] swap getf
        call
        leave
    resolve
    pcall*/
    
    PNODE* call_rest = ncons(vm, dup);
    register_func(vm, call_rest, "call_rest", 0);
    call_stub->into = call_rest;
    
    ncons(vm, fpush);
    tmp = ncons(vm, NULL);
    tmp->value = pcall_val;//func_value(vm, pcall);
    ncons(vm, eq);
    
    ncons(vm, cjump);
    mark(vm);
        ncons(vm, dup);
        ncons(vm, meta);
        
        ncons(vm, fpush);
        tmp = ncons(vm, NULL);
        tmp->value = symbol_value("call");
        ncons(vm, swap);
        ncons(vm, getf);
        ncons(vm, call);
        ncons(vm, jump);
        mark(vm);
    resolve(vm, 2);
        ncons(vm, pcall);
    resolve(vm, 1);
    
    ncons(vm, leave);
    
    
    run = register_func(vm, defun(vm ,2, call, exit), "run", 0);
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

typedef enum {
    TOK_WORD, TOK_NUM, TOK_END
}TOK_TYPE;

char const* tokenizer_pos;

void init_tokenizer(char const* str) {
    tokenizer_pos = str;
}

TOK_TYPE next_tok(char* tok) {
    char* tok_pos = tok;
    
    start:
        if(*tokenizer_pos == 0)
            return TOK_END;
        else if(isspace(*tokenizer_pos)) {
            ++tokenizer_pos;
            goto start;
        }else if(isdigit(*tokenizer_pos)) {
            *(tok_pos++) = *(tokenizer_pos++);
            goto word_or_num;
        }else {
            assert(isprint(*tokenizer_pos));
            *(tok_pos++) = *(tokenizer_pos++);
            goto word;
        }
    word:
        if(*tokenizer_pos == 0 || isspace(*tokenizer_pos)) {
            *(tok_pos++) = 0;
            return TOK_WORD;
        }else {
            assert(isprint(*tokenizer_pos));
            *(tok_pos++) = *(tokenizer_pos++);
            goto word;
        }
    word_or_num:
        if(*tokenizer_pos == 0 || isspace(*tokenizer_pos)) {
            *(tok_pos++) = 0;
            return TOK_NUM;
        }else if(isdigit(*tokenizer_pos)) {
            *(tok_pos++) = *(tokenizer_pos++);
            goto word_or_num;
        }else {
            assert(isprint(*tokenizer_pos));
            *(tok_pos++) = *(tokenizer_pos++);
            goto word;
        }
}

VALUE parse_num(char const* str) {
    VALUE v = {.type = NUM_TYPE, .data.num = strtod(str, NULL)};
    return v;
}

VALUE parse_word(char const* str) {
    VALUE v = {.type = SYMBOL_TYPE, .data.obj = (OBJECT_BASE*)get_symbol(str)};
    return v;
}

void eval_str(VM* vm, char const* str) {
    init_tokenizer(str);
    char tok[256];
    TOK_TYPE tt;
    VALUE key, item;
    
    for(tt = next_tok(tok); tt != TOK_END; tt = next_tok(tok)) {
        switch(tt) {
            case TOK_WORD:
                key = symbol_value(tok);
                map_get(&item, &global_scope->map, &key);
                
                if(value_is_nil(&item)) {
                    printf("Undefined function '%s'.\n", tok);
                }else if(item.type != FUNC_TYPE) {
                    printf("'%s' is not a function.\n", tok);
                }else {
                    push(vm, item);
                    vm->curr = run;
                    next(vm);
                    loop(vm);
                }
                break;
            case TOK_NUM:
                push(vm, parse_num(tok));
                break;
        }
        
        
    }
    
    print_debug_info(vm);
}

int main() {
    VM* vm = create_vm();
    
    init(vm);
    char str[256];
    
    while(1) {
        gets(str);
        eval_str(vm, str);
    }
    
    
    //eval_str("+");
    
    
    
    

    
    //run = register_func(vm, defun(2, dcall, exit), "run", 0);
    
    // |---->
    // key obj
    /*
    dup default_meta eq
    cjump mark
        dup meta [ index ] swap getf
        call
        leave
    resolve
        dgetf
    */
    

//    eval("          foo       1234 sd 132d324 31 ");
    
    
    
    //printf(dbl->
    
    /*
    PNODE* quad = defun(2, dbl, dbl);
    PNODE* main = defun(5, dcall, plus, drop, swap, dgetf);*/
    
    
    
    
    /*test111
    OBJECT* o = create_object();
    VALUE key = symbol_value("foo");
    VALUE val = num_value(123);
    //VALUE key2 = symbol_value("foo");
    map_put(&o->map, &key, &val);
    push(vm, key);
    push(vm, (VALUE){.type = OBJECT_TYPE, .data.obj = (OBJECT_BASE*)o});*/
    
    
    
    
    //push(vm, num_value(2));
    //push(vm, func_value(vm, dbl, 0));
    
    
    
    
    
    /*
    push(vm, num_value(2));
    push(vm, num_value(4));
    push(vm, func_value(vm, quad));*/
    
    /*
    VALUE v;
    v.type = BOOL_TYPE;
    v.data.boolean = 0;
    push(vm, num_value(2));
    push(vm, num_value(3));
    push(vm, v);*/
    
    
    /*
    OBJECT* o = create_object();
    VALUE key = symbol_value("index");
    VALUE val = num_value(123);
    map_put(&o->map, &key, &val);
    push(vm, key);
    push(vm, (VALUE){.type = OBJECT_TYPE, .data.obj = (OBJECT_BASE*)o});
    */
    
    
    
    //|---------->
    //key obj
        /*
    
    dup meta
    dup default_meta eq
    cjump mark
        [ index ] swap getf
        call
        leave
    resolve
        dgetf*/
    
    
    
    

    
    // *******
    // * etc *
    // *******
    
    
    
    
    //PNODE* main = defun(4, meta, swap, dgetf, f);
    
    /*                    
    PNODE* main = defun(1, call);
    
    
    
    
    
    
                        
    push(vm, func_value(vm, main, 0));
    
    vm->curr = run;
    next(vm);
    loop();*/
    
    getch();
    return 0;
}
