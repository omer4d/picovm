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

PNODE* ncons(PNODE* n) {
    PNODE* node = program_pos;
    ++program_pos;
    node->into = n;
    return node;
}

PNODE* fcons(CFUN fp, char const* debug) {
    PNODE* node = program_pos;
    ++program_pos;
    node->fp = fp;
    debug_info[node - program] = debug;
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

void resolve() {
    assert(mark_sp > mark_stack);
    --mark_sp;
    mark_sp->into->into = program_pos;
}

PNODE* mark_placeholder;
PNODE* resolve_placeholder;

PNODE* defun(char const* name, int n, ...) {
    static PNODE* leave = NULL;
    
    if(!leave) {
        leave = fcons(leave_impl, "leave");
    }
    
    va_list argp;
    va_start(argp, n);
    
    PNODE* first = fcons(enter_impl, "enter");
    debug_info[first - program] = name;
    
    int i;
    for(i = 0; i < n; ++i) {
        PNODE* pn = va_arg(argp, PNODE*);
        if(pn == mark_placeholder)
            mark();
        else if(pn == resolve_placeholder)
            resolve();
        else
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

VALUE symbol_value(char const* name) {
    VALUE v;
    v.type = SYMBOL_TYPE;
    v.data.obj = (OBJECT_BASE*)create_symbol(name);
    return v;
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
    assert(v.type == FUNC_TYPE);
    curr = ((FUNC*)v.data.obj)->pnode;
    next();
}

void dgetf_impl() {
    VALUE key = pop();
    VALUE objval = pop();
    assert(objval.type == OBJECT_TYPE);
    OBJECT* obj = (OBJECT*)objval.data.obj;
    VALUE val;
    map_get(&val, &obj->map, &key);
    push(val);
    next();
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
    v.type = FUNC_TYPE;
    v.data.obj = (OBJECT_BASE*)create_func(pnode);
    return v;
}

void init() {
    mark_placeholder = malloc(sizeof(PNODE));
    resolve_placeholder = malloc(sizeof(PNODE));
    
    OBJECT* meta = create_object();
    meta->base.meta = meta;
    MAP* m = &meta->map;
    //map_put(m, 
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
        //print_debug_info();
        //getch();
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

int main() {
    init();
    
    PNODE* dup = fcons(dup_impl, "dup");
    PNODE* swap = fcons(swap_impl, "swap");
    PNODE* drop = fcons(drop_impl, "drop");
    PNODE* plus = fcons(plus_impl, "plus");
    PNODE* dcall = fcons(dcall_impl, "dcall");
    PNODE* dgetf = fcons(dgetf_impl, "dgetf");
    PNODE* cjump = fcons(cjump_impl, "cjump");
    PNODE* jump = fcons(jump_impl, "jump");
    PNODE* exit = fcons(exit_impl, "exit");
    
    PNODE* run = defun("run", 2, dcall, exit);
    
    
    init_tokenizer("          foo       1234 sd 132d324 31 ");
    char tok[256];
    TOK_TYPE tt;
    for(tt = next_tok(tok); tt != TOK_END; tt = next_tok(tok)) {
        switch(tt) {
            case TOK_WORD:
                printf("WORD: %s\n", tok);
                break;
            case TOK_NUM:
                printf("NUM: %s\n", tok);
                break;
        }
    }

//    eval("          foo       1234 sd 132d324 31 ");
    
    /*
    PNODE* dbl = defun("dbl", 2, dup, plus);
    PNODE* quad = defun("quad", 2, dbl, dbl);
    PNODE* main = defun("main", 5, dcall, plus, drop, swap, dgetf);
    
    OBJECT* o = create_object();
    VALUE key = symbol_value("foo");
    VALUE val = num_value(123);
    //VALUE key2 = symbol_value("foo");
    map_put(&o->map, &key, &val);
    push(key);
    push((VALUE){.type = OBJECT_TYPE, .data.obj = (OBJECT_BASE*)o});
    
    push(num_value(2));
    push(num_value(4));
    push(func_value(quad));*/
    
    
    VALUE v;
    v.type = BOOL_TYPE;
    v.data.boolean = 0;
    push(num_value(2));
    push(v);
    
    
    PNODE* main = defun("main", 8,
                        cjump, mark_placeholder,
                        dup,
                        jump, mark_placeholder,
                        resolve_placeholder,
                        dup,
                        resolve_placeholder);
    push(func_value(main));
    
    curr = run;
    next();
    loop();
    
    getch();
    return 0;
}
