#ifndef __COMPILER_H__
#define __COMPILER_H__

#include <stdio.h>
#include "value.h"
#include "pnode.h"

#define PROGRAM_STACK_SIZE 256

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
    ANODE* spare;
    ANODE* last;
    int size; // number of PNODES (an ANODE may translate into more than one PNODE)
}PROGRAM;

typedef struct DEBUG_ENTRY_t {
    PNODE* start;
    int len;
    char const* context_name;
}DEBUG_ENTRY;

typedef struct COMPILER_t {
    PROGRAM program_stack[PROGRAM_STACK_SIZE];
    PROGRAM* program_sp;
    
    int debug_entry_num;
    DEBUG_ENTRY debug_entries[2000];
}COMPILER;

void init_compiler(COMPILER* c);
void cleanup_compiler(COMPILER* c);
void reset_compiler(COMPILER* c);

void begin_compilation(COMPILER* c);
void abort_compilation(COMPILER* c);
PNODE* end_compilation(COMPILER* c, char const* context_name);

void compile_call(COMPILER* c, PNODE const* into);
void compile_literal(COMPILER* c, VALUE v);
ANODE* compile_cjump(COMPILER* c);
ANODE* compile_jump(COMPILER* c);
void compile_recur(COMPILER* c);
void compile_stub(COMPILER* c);

ANODE* compiler_pos(COMPILER* c);

void resolve_jump(ANODE* jnode, ANODE* dest);

int compiler_is_compiling(COMPILER* c);

char const* find_compilation_context(COMPILER* c, PNODE const* n);

#endif
