#ifndef __COMPILER_H__
#define __COMPILER_H__

#include <stdio.h>
#include "value.h"
#include "pnode.h"
#include "program_writer.h"

#define PW_STACK_SIZE 256

typedef struct COMPILER_t {
    FILE* in;
    PROGRAM_WRITER* pw_stack;
    PROGRAM_WRITER* pw_sp;
}COMPILER;

void init_compiler(COMPILER* c);
void cleanup_compiler(COMPILER* c);

void begin_compilation(COMPILER* c);
PNODE* end_compilation(COMPILER* c);

void compile_func_enter(COMPILER* c);
void compile_call(COMPILER* c, PNODE const* into);
void compile_literal(COMPILER* c, VALUE v);
void compile_cjump(COMPILER* c);
void compile_jump(COMPILER* c);
void compile_recur(COMPILER* c);
void compile_stub(COMPILER* c);
void compiler_resolve(COMPILER* c, int mark_id);

#endif
