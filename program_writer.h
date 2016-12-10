#ifndef __PROGRAM_WRITER_H__
#define __PROGRAM_WRITER_H__

#include "value.h"
#include "pnode.h"

#define MARK_STACK_SIZE 256

typedef struct PROGRAM_WRITER_t {
    PNODE* mark_stack;
    PNODE* mark_sp;
    
    PNODE* data;
    PNODE* write_pos;
    int cap;
}PROGRAM_WRITER;

void init_program_writer(PROGRAM_WRITER* p);
void cleanup_program_writer(PROGRAM_WRITER* p);

void program_writer_mark(PROGRAM_WRITER* p);
void program_writer_resolve(PROGRAM_WRITER* p, int n);
void program_writer_drop_marks(PROGRAM_WRITER* p, int n);

void write_cfun(PROGRAM_WRITER* p, CFUN fp);
void write_pnode(PROGRAM_WRITER* p, PNODE const* n);
void write_value(PROGRAM_WRITER* p, VALUE value);

PNODE* acquire_program(PROGRAM_WRITER* p);

#endif
