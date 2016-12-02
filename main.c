#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>

#include "value.h"
#include "types.h"

#define ARG_STACK_SIZE 1024
#define RET_STACK_SIZE 1024
#define MARK_STACK_SIZE 256

typedef void (*CFUN)();

typedef union PNODE_t {
    union PNODE_t* into;
    CFUN fp;
    VALUE value;
}PNODE;

PNODE program[3*1000*1000];
char const* debug_info[3*1000*1000] = {0};
PNODE* program_pos = program;

VALUE arg_stack[ARG_STACK_SIZE] = {0};
VALUE* arg_sp = arg_stack;

PNODE* ret_stack[RET_STACK_SIZE] = {0};
PNODE** ret_sp = ret_stack;

PNODE mark_stack[MARK_STACK_SIZE];
PNODE* mark_sp = mark_stack;

PNODE* curr;
void (*instr)();

void set_debug_info(PNODE* node, char const* debug) {
    debug_info[node - program] = debug;
}

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

void swap_impl() {
    VALUE a = pop();
    VALUE b = pop();
    push(a);
    push(b);
    next();
}

void drop_impl() {
    pop();
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

void jump_impl() {
    ++curr;
	curr = curr->into;
	instr = curr->into->fp;
}

void cjump_impl() {
    VALUE c = pop();
    assert(c.type == BOOL_TYPE);
	if(c.data.boolean) {
        ++curr;
	    curr = curr->into;
	    instr = curr->into->fp;
	}
	
	else {
		++curr;
		next();
	}
}

void mark() {
    assert(mark_sp < mark_stack + MARK_STACK_SIZE);
    PNODE* pn = ncons(mark_sp);
    mark_sp->into = pn;
    ++mark_sp;
}

void resolve(int n) {
    assert(mark_sp - n >= mark_stack);
    //--mark_sp;
    (mark_sp - n)->into->into = program_pos;
}

void drop_marks(int n) {
    mark_sp -= n;
}

PNODE* mark_placeholder;
PNODE* resolve_placeholder;

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
        PNODE* pn = va_arg(argp, PNODE*);
        //if(pn == mark_placeholder)
        //    mark();
        //else if(pn == resolve_placeholder)
        //    resolve();
        //else
            ncons(pn);
    }
    
    va_end(argp);
    ncons(leave);
    
    return first;
}

typedef struct SYMBOL_t {
    OBJECT_BASE base;
    char const* name;
}SYMBOL;

OBJECT* sym_meta = NULL;

SYMBOL* get_symbol(char const* name) {
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
        sym_table_cap *= 2;
        sym_table = realloc(sym_table, sizeof(SYMBOL*) * sym_table_cap);
    }
    
    SYMBOL* sym = malloc(sizeof(SYMBOL));
    sym->base.meta = sym_meta;
    sym->name = name;
    sym_table[sym_num++] = sym;
    return sym;
}

VALUE symbol_value(char const* name) {
    VALUE v;
    v.type = SYMBOL_TYPE;
    v.data.obj = (OBJECT_BASE*)get_symbol(name);
    return v;
}

typedef struct FUNC_t {
    OBJECT_BASE base;
    union PNODE_t* pnode;
}FUNC;

void dcall_impl() {
    assert(ret_sp <= &ret_stack[RET_STACK_SIZE - 1]);
    *ret_sp = curr;
    ++ret_sp;
    VALUE v = pop();
    assert(v.type == FUNC_TYPE);
    curr = ((FUNC*)v.data.obj)->pnode;
    next();
}

char const* lookup_debug_info(PNODE*);

void pcall_impl() {
    assert(ret_sp <= &ret_stack[RET_STACK_SIZE - 1]);
    VALUE v = pop();
    assert(v.type == FUNC_TYPE);
    instr = ((FUNC*)v.data.obj)->pnode->fp;
}

void dgetf_impl() {
    VALUE objval = pop();
    VALUE key = pop();
    assert(objval.type == OBJECT_TYPE);
    OBJECT* obj = (OBJECT*)objval.data.obj;
    VALUE val;
    map_get(&val, &obj->map, &key);
    push(val);
    next();
}

void meta_impl() {
    VALUE objval = pop();
    assert(objval.type == OBJECT_TYPE || objval.type == FUNC_TYPE);
    assert(!value_is_nil(&objval));
    OBJECT* obj = (OBJECT*)objval.data.obj;
    objval.type = OBJECT_TYPE;
    objval.data.obj = (OBJECT_BASE*)obj->base.meta;
    push(objval);
    next();
}

void push_impl() {
    push((++curr)->value);
    next();
}

void eq_impl() {
    VALUE a = pop();
    VALUE b = pop();
    VALUE c = {.type = BOOL_TYPE, .data.boolean = values_equal(&a, &b) ? 1 : 0};
    push(c);
    next();
}

PNODE* pcall;
PNODE* dcall;
VALUE pcall_val;
VALUE dcall_val;

VALUE func_value(PNODE* pnode, int primitive);
PNODE* register_func(PNODE*, char const*, int primitive);
OBJECT* func_meta = NULL;
OBJECT* primitive_func_meta = NULL;

FUNC* create_func(PNODE* pnode, int primitive) {
    FUNC* f = malloc(sizeof(FUNC));
    
    if(!func_meta) {
        func_meta = create_object();
        primitive_func_meta = create_object();
        
        dcall = register_func(fcons(dcall_impl), "dcall", 1);
        pcall = register_func(fcons(pcall_impl), "pcall", 1);
        

        pcall_val = func_value(pcall, 1);
        VALUE key = symbol_value("call");
        VALUE item = pcall_val;
        map_put(&primitive_func_meta->map, &key, &item);
        
        
        dcall_val = func_value(dcall, 1);
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

VALUE func_value(PNODE* pnode, int primitive) {
    VALUE v;
    v.type = FUNC_TYPE;
    v.data.obj = (OBJECT_BASE*)create_func(pnode, primitive);
    return v;
}











OBJECT* global_scope;

PNODE* register_func(PNODE* pnode, char const* name, int primitive) {
    VALUE key = symbol_value(name);
    VALUE item = func_value(pnode, primitive);
    map_put(&global_scope->map, &key, &item);
    set_debug_info(pnode, name);
    return pnode;
}

PNODE* run = NULL;

void set_method(OBJECT* object, char const* name, PNODE* func, int primitive) {
    VALUE key = symbol_value(name);
    VALUE item = func_value(func, primitive);
    map_put(&object->map, &key, &item);
}

void compile_push(VALUE val) {
    PNODE* tmp = ncons(NULL);
    tmp->value = val;
}

void init() {
    mark_placeholder = malloc(sizeof(PNODE));
    resolve_placeholder = malloc(sizeof(PNODE));
    
    OBJECT* meta = create_object();
    meta->base.meta = meta;
    init_object_system(meta);
    
    global_scope = create_object();
    
    PNODE* dcall = register_func(fcons(dcall_impl), "dcall", 1);
    PNODE* exit = register_func(fcons(exit_impl), "exit", 1);
    
    run = register_func(defun(2, dcall, exit), "run", 0);
    
    PNODE* dgetf = register_func(fcons(dgetf_impl), "dgetf", 1);
    set_method(meta, "index", dgetf, 1);
    
    
    /*
    PNODE* getf = fcons(enter_impl);
    VALUE meta_val;
    meta_val.type = OBJECT_TYPE;
    meta_val.data.obj = (OBJECT_BASE*)meta;
    
    ncons(dup);
    compile_push(meta_val);
    ncons(push);
    ncons(eq);
    ncons(*/
    
    /*
    dup default_meta eq
    cjump mark
        dup meta [ index ] swap getf
        call
        leave
    resolve
        dgetf*/
}

char const* lookup_debug_info(PNODE* pnode) {
    int idx = pnode - program;
    return idx >= 0 ? (debug_info[idx] ? debug_info[idx] : "<no-info>") : "<invalid>";
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
            sprintf(str, "<function %s>", lookup_debug_info(((FUNC*)sp->data.obj)->pnode));
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



void print_debug_info() {
    VALUE* asp;
    PNODE** rsp;
    char tmp[256] = {};
    
    printf("Next instruction: %s\n\n", curr ? lookup_debug_info(curr->into) : "N/A");
    printf("%-30s %-30s\n", "ARG STACK", "CALL STACK");
    printf("_________________________________________\n");
    for(asp = arg_stack, rsp = ret_stack; asp < arg_sp || rsp < ret_sp; ++rsp, ++asp) {
        if(rsp < ret_sp && asp < arg_sp)
            printf("%-30s %-30s\n", value_to_string(tmp, asp), lookup_debug_info((*rsp)->into));
        else if(rsp < ret_sp)
            printf("%-30s %-30s\n", "", lookup_debug_info((*rsp)->into));
        else
            printf("%-30s %-30s\n", value_to_string(tmp, asp), "");
    }
    
    printf("\n\n\n\n");
}

void loop() {
    while(instr) {
        print_debug_info();
        getch();
        instr();
    }
    
    print_debug_info();
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

void eval_str(char const* str) {
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
                    push(item);
                    curr = run;
                    next();
                    loop();
                }
                break;
            case TOK_NUM:
                push(parse_num(tok));
                break;
        }
        
        print_debug_info();
    }
}

int main() {
    init();
    
    //eval_str("555 4 +");
    //eval_str("+");
    
    
    
    
    PNODE* dup = register_func(fcons(dup_impl), "dup", 1);
    PNODE* swap = register_func(fcons(swap_impl), "swap", 1);
    PNODE* drop = register_func(fcons(drop_impl), "drop", 1);
    PNODE* plus = register_func(fcons(plus_impl), "+", 1);
    PNODE* dcall = register_func(fcons(dcall_impl), "dcall", 1);
   
    
    PNODE* meta = register_func(fcons(meta_impl), "meta", 1);
    PNODE* dgetf = register_func(fcons(dgetf_impl), "dgetf", 1);
    
    PNODE* cjump = register_func(fcons(cjump_impl), "cjump", 1);
    PNODE* jump = register_func(fcons(jump_impl), "jump", 1);
    
    PNODE* exit = register_func(fcons(exit_impl), "exit", 1);
    
    PNODE* leave = register_func(fcons(leave_impl), "leave", 1);
    PNODE* fpush = register_func(fcons(push_impl), "push", 1);
    PNODE* eq = register_func(fcons(eq_impl), "eq", 1);
    
    run = register_func(defun(2, dcall, exit), "run", 0);
    
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
    
    
    PNODE* dbl = register_func(defun(2, dup, plus), "dbl", 0);
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
    push(key);
    push((VALUE){.type = OBJECT_TYPE, .data.obj = (OBJECT_BASE*)o});*/
    
    
    
    
    push(num_value(2));
    push(func_value(dbl, 0));
    
    
    
    
    
    /*
    push(num_value(2));
    push(num_value(4));
    push(func_value(quad));*/
    
    /*
    VALUE v;
    v.type = BOOL_TYPE;
    v.data.boolean = 0;
    push(num_value(2));
    push(num_value(3));
    push(v);*/
    
    
    /*
    OBJECT* o = create_object();
    VALUE key = symbol_value("index");
    VALUE val = num_value(123);
    map_put(&o->map, &key, &val);
    push(key);
    push((VALUE){.type = OBJECT_TYPE, .data.obj = (OBJECT_BASE*)o});
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
    
    
    
    
    PNODE* call = fcons(enter_impl);
    register_func(call, "call", 0);
    ncons(jump);
    PNODE* call_stub = ncons(NULL);
    
    // ********
    // * getf *
    // ********
    PNODE* getf = fcons(enter_impl);
    register_func(getf, "getf", 0);

    ncons(dup);
    ncons(meta);
    
    ncons(dup);
    ncons(fpush);
    PNODE* tmp = ncons(NULL);
    tmp->value.type = OBJECT_TYPE;
    tmp->value.data.obj = (OBJECT_BASE*)default_meta;
    ncons(eq);
    
    ncons(cjump);
    mark();
        ncons(fpush);
        tmp = ncons(NULL);
        tmp->value = symbol_value("index");
        ncons(swap);
        ncons(getf);
        ncons(call);
        ncons(jump);
        mark();
    resolve(2);
        ncons(drop);
        ncons(dgetf);
    resolve(1);
    drop_marks(2);
    
    ncons(leave);
    
    
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
    
    PNODE* call_rest = ncons(dup);
    register_func(call_rest, "call_rest", 0);
    call_stub->into = call_rest;
    
    ncons(fpush);
    tmp = ncons(NULL);
    tmp->value = pcall_val;//func_value(pcall);
    ncons(eq);
    
    ncons(cjump);
    mark();
        ncons(dup);
        ncons(meta);
        
        ncons(fpush);
        tmp = ncons(NULL);
        tmp->value = symbol_value("call");
        ncons(swap);
        ncons(getf);
        ncons(call);
        ncons(jump);
        mark();
    resolve(2);
        ncons(pcall);
    resolve(1);
    
    ncons(leave);
    
    // *******
    // * etc *
    // *******
    
    
    
    
    //PNODE* main = defun(4, meta, swap, dgetf, f);
                        
    PNODE* main = defun(1, call);
    
    
    
    
    
    
                        
    push(func_value(main, 0));
    
    curr = run;
    next();
    loop();
    
    getch();
    return 0;
}
