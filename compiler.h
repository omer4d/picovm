#ifndef __COMPILER_H__
#define __COMPILER_H__

#include <stdio.h>
#include "value.h"
#include "pnode.h"

#define PROGRAM_STACK_SIZE 256
#define MARK_STACK_SIZE 256

typedef enum {
    ANODE_CALL_FUNC, ANODE_CALL_PRIMITIVE, ANODE_RECUR, ANODE_JUMP, ANODE_CJUMP, ANODE_LONGJUMP, ANODE_LITERAL,
}ANODE_TYPE;

typedef struct ANODE_t {
    ANODE_TYPE type;
    struct ANODE_t* next;
    PNODE* data_dest;
    union {
        PNODE const* into;
        struct ANODE_t* target;
        VALUE value;
    }data;
}ANODE;

typedef struct PROGRAM_t {
    ANODE* first;
    ANODE* last;
    int size; // number of PNODES (an ANODE may translate into more than one PNODE)
}PROGRAM;

typedef struct COMPILER_t {
    FILE* in;
    ANODE* mark_stack;
    ANODE* mark_sp;
    ANODE* last_resolve_request;
    PROGRAM* program_stack;
    PROGRAM* program_sp;
}COMPILER;

void init_compiler(COMPILER* c);
void cleanup_compiler(COMPILER* c);

void begin_compilation(COMPILER* c);
PNODE* end_compilation(COMPILER* c);

void compile_call(COMPILER* c, PNODE const* into);
void compile_literal(COMPILER* c, VALUE v);
void compile_cjump(COMPILER* c);
void compile_jump(COMPILER* c);
void compile_recur(COMPILER* c);
void compile_stub(COMPILER* c);
void compiler_resolve(COMPILER* c, int mark_id);
void compiler_drop_marks(COMPILER* c, int n);

#endif
