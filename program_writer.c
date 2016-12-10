#include <malloc.h>
#include "program_writer.h"
#include "macros.h"
#include "primitives.h"

int decide_next_op(PNODE* pn) {
    CFUN fp = pn->into->fp;
    
    if(fp == primitives[push_loc].fp)
        return 1;
    else if(fp == primitives[jump_loc].fp || fp == primitives[cjump_loc].fp)
        return 2;
    else
        return 0;
}

void relocate_branches(PROGRAM_WRITER* p, PNODE* old_start, int old_len) {
    int i, op = 1; // 0 = test, 1 = skip, 2 = relocate
    
    for(i = 0; i < old_len; ++i) {
        PNODE* n = p->data + i;
        
        switch(op) {
            case 0:
                op = decide_next_op(n);
                break;
            case 1:
                op = 0;
                break;
            case 2:
                // is it a local branch?
                if(n->into >= old_start && n->into < old_start + old_len)
                    n->into = n->into - old_start + p->data;
                // is it a mark?
                else if(n->into >= p->mark_stack && n->into < p->mark_sp) {
                    ((PNODE*)n->into)->into = n; // fix mark to point at copied pnode
                }
                // else it was an absolute address
                op = 0;
                break;
        }
    }
}

PNODE* next_pnode(PROGRAM_WRITER* p) {
    int old_len = program_len(p);
    PNODE* old_data = p->data;
    
    if(old_len == p->cap) {
        printf("wtf!");
        p->cap = p->cap * 3 / 2;
        p->data = realloc(p->data, p->cap);
        relocate_branches(p, old_data, old_len);
        p->write_pos = p->data + old_len;
    }
    
    return (p->write_pos)++;
}

int program_len(PROGRAM_WRITER* p) {
    return p->write_pos - p->data;
}

void init_program_writer(PROGRAM_WRITER* p) {
    p->mark_stack = calloc(MARK_STACK_SIZE, sizeof(PNODE));
    p->mark_sp = p->mark_stack;
    
    p->cap = 200;
    p->data = malloc(sizeof(PNODE) * p->cap);
    p->write_pos = p->data;
}

void cleanup_program_writer(PROGRAM_WRITER* p) {
    free(p->mark_sp);
    free(p->data);
}


void write_cfun(PROGRAM_WRITER* p, CFUN fp) {
    PNODE* n = next_pnode(p);
    n->fp = fp;
}

void write_pnode(PROGRAM_WRITER* p, PNODE const* into) {
    PNODE* n = next_pnode(p);
    n->into = into;
}

void write_value(PROGRAM_WRITER* p, VALUE value) {
    PNODE* n = next_pnode(p);
    n->value = value;
}

void program_writer_mark(PROGRAM_WRITER* p) {
    ASSERT_PUSH(p->mark_stack, p->mark_sp, MARK_STACK_SIZE);
    
    PNODE* n = next_pnode(p);
    n->into = p->mark_sp;
    p->mark_sp->into = n;
    ++p->mark_sp;
}

void program_writer_resolve(PROGRAM_WRITER* p, int n) {
    assert(n < 0);
    assert(p->mark_sp + n >= p->mark_stack);
    assert(program_len(p) > 0);
    assert((p->mark_sp + n)->into);
    
    ((PNODE*)(p->mark_sp + n)->into)->into = p->write_pos - 1;
    (p->mark_sp + n)->into = NULL;
}

void program_writer_drop_marks(PROGRAM_WRITER* p, int n) {
    assert(p->mark_sp - n >= p->mark_stack);
    p->mark_sp -= n;
}

PNODE* acquire_program(PROGRAM_WRITER* p) {
    PNODE* tmp = p->data;
    p->data = NULL;
    return tmp;
}
