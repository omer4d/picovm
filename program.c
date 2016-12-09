#include "program.h"
#include "primitives.h"

void init_program(PROGRAM* p, PNODE* data) {
    p->data = data;
    p->write_pos = data;
}

void cleanup_program(PROGRAM* p) {
    free(p->data);
}

int decide_next_op(PNODE* pn) {
    CFUN fp = pn->into->fp;
    
    if(fp == push_impl)
        return 1;
    else if(fp == jump_impl || fp == cjump_impl)
        return 2;
    else
        return 0;
}

void copy_program(PNODE* dest, PROGRAM* src_prog, PNODE* new_base, PNODE* mark_stack, PNODE* mark_sp) {
    int i, op = 1; // 0 = copy once, 1 = copy twice, 2 = relocate
    PNODE* src = src_prog->data;
    
    for(i = 0; i < program_len(src_prog); ++i) {
        dest[i] = src[i];
        
        switch(op) {
            case 0:
                op = decide_next_op(src + i);
                break;
            case 1:
                op = 0;
                break;
            case 2:
                // is it a local branch?
                if(src[i].into >= src_prog->data && src[i].into < src_prog->write_pos)
                    dest[i].into = src[i].into - src + new_base;
                // is it a mark?
                else if(src[i].into >= mark_stack && src[i].into < mark_sp) {
                    src[i].into->into = &dest[i]; // fix mark to point at copied pnode
                }
                // else it was an absolute address
                op = 0;
                break;
        }
    }
}

int program_len(PROGRAM* p) {
    return p->write_pos - p->data;
}

PNODE* next_pnode(PROGRAM* p) {
    return (p->write_pos)++;
}
