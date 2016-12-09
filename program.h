#ifndef __PROGRAM_H__
#define __PROGRAM_H__

#include "value.h"

struct VM_t;

typedef void (*CFUN)(struct VM_t*);

typedef union PNODE_t {
    union PNODE_t* into;
    CFUN fp;
    VALUE value;
}PNODE;

typedef struct {
    PNODE* data;
    PNODE* write_pos;
}PROGRAM;

void init_program(PROGRAM* p, PNODE* data);
void cleanup_program(PROGRAM* p);
//void copy_program(PNODE* dest, PNODE* src);
int program_len(PROGRAM* p);
PNODE* next_pnode(PROGRAM* p);

#endif
