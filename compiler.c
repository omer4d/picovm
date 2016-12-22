#include "compiler.h"
#include "macros.h"
#include "object.h"
#include "tokenizer.h"
#include "primitives.h"

#include <malloc.h>

PROGRAM* curr_program(COMPILER* c) {
    assert(c->program_sp > c->program_stack);
    return c->program_sp - 1;
}

void init_program(PROGRAM* p) {
    p->first = NULL;
    p->spare = malloc(sizeof(ANODE));
    p->last = NULL;
    p->size = 0;
}

void cleanup_program(PROGRAM* p) {
    ANODE* n;
    for(n = p->first; n != NULL;) {
        ANODE* next = n->next;
        free(n);
        n = next;
    }
    free(p->spare);
}

ANODE* next_anode(COMPILER* c, ANODE_TYPE type) {
    PROGRAM* p = curr_program(c);
    ANODE* n = p->spare;
    p->spare = malloc(sizeof(ANODE));
    
    n->type = type;
    n->next = NULL;
    n->data_dest = NULL;
    
    if(!p->last) {
        p->first = n;
        p->last = n;
    }else {
        p->last->next = n;
        p->last = n;
    }
    
    p->size += (type == ANODE_JUMP || ANODE_CJUMP || ANODE_LITERAL ? 2 : 1);
    
    return n;
}

void init_compiler(COMPILER* c) {
    c->program_sp = c->program_stack;
    
    c->debug_entry_num = 0;
}

void cleanup_compiler(COMPILER* c) {
    PROGRAM* p;
    for(p = c->program_stack; p < c->program_sp; ++p)
        cleanup_program(p);
}

void reset_compiler(COMPILER* c) {
    cleanup_compiler(c);
    init_compiler(c);
}

int unfinished_compilation_count(COMPILER* c) {
    return c->program_sp - c->program_stack;
}

void begin_compilation(COMPILER* c) {
    ASSERT_PUSH(c->program_stack, c->program_sp, PROGRAM_STACK_SIZE);
    init_program(c->program_sp++);
}

int is_special_primitive(PNODE const* pnode) {
    return pnode == &primitives[jump_loc] ||
            pnode == &primitives[cjump_loc] ||
            pnode == &primitives[push_loc];
}

int is_primitive(PNODE const* pnode) {
    return pnode >= primitives && pnode < primitives + PRIMITIVE_NUM;
}

int is_leave(ANODE* n) {
    if(n) {
        if(n->type == ANODE_CALL_PRIMITIVE && n->data.into == &primitives[leave_loc])
            return 1;
        else if(n->type == ANODE_JUMP)
            return is_leave(n->data.target);
        else
            return 0;
    }
    return 0;
}

void perform_tco(PROGRAM* prog) {
    ANODE* n;
    for(n = prog->first; n != NULL; n = n->next) {
        if(n->type == ANODE_CALL_FUNC && is_leave(n->next)) {
            n->type = ANODE_LONGJUMP;
            ++n->data.into;
            ++prog->size;
        }
        
        if(n->type == ANODE_RECUR && is_leave(n->next)) {
            n->type = ANODE_JUMP;
            n->data.target = prog->first;
            ++prog->size;
        }
    }
}

void drop_compilation(COMPILER* c) {
    ASSERT_POP(c->program_stack, c->program_sp);
    cleanup_program(--c->program_sp);
}

PNODE* end_compilation(COMPILER* c, char const* context_name) {
    ASSERT_POP(c->program_stack, c->program_sp);
    PROGRAM* prog = curr_program(c);
    
    //perform_tco(prog);
    
    PNODE* out = malloc(sizeof(PNODE) * prog->size + 1);
    PNODE* write_pos = out + 1;
    
    out[0].fp = primitives[enter_loc].fp;
    ANODE* n;
    for(n = prog->first; n != NULL; n = n->next) {
        n->data_dest = write_pos;
        switch(n->type) {
            case ANODE_CALL_FUNC: case ANODE_CALL_PRIMITIVE: case ANODE_RECUR:
                (write_pos++)->into = n->data.into;
                break;
            case ANODE_JUMP:
                (write_pos++)->into = &primitives[jump_loc];
                (write_pos++)->into = NULL;
                break;
            case ANODE_CJUMP:
                (write_pos++)->into = &primitives[cjump_loc];
                (write_pos++)->into = NULL;
                break;
            case ANODE_LONGJUMP:
                (write_pos++)->into = &primitives[jump_loc];
                (write_pos++)->into = n->data.into;
                break;
            case ANODE_LITERAL:
                (write_pos++)->into = &primitives[push_loc];
                (write_pos++)->value = n->data.value;
                break;
            default:
                assert("unidentified ANODE type!" && 0);
        }
    }
    
    for(n = prog->first; n != NULL; n = n->next) {
        if(n->type == ANODE_JUMP || n->type == ANODE_CJUMP) {
            (n->data_dest + 1)->into = n->data.target->data_dest;
        }else if(n->type == ANODE_RECUR) {
            n->data_dest->into = out;
        }
    }
    
    DEBUG_ENTRY* d = &c->debug_entries[c->debug_entry_num++];
    d->start = out;
    d->len = prog->size + 1;
    d->context_name = context_name;
    
    //printf("context_name %s, %d %d\n", d->context_name, (int)d->start, (int)(d->start + d->len) - 1);
    
    cleanup_program(--c->program_sp);
    return out;
}

char const* find_compilation_context(COMPILER* c, PNODE const* n) {
    int i;
    char const* context_name = "unknown";
    
    for(i = 0; i < c->debug_entry_num; ++i) {
        DEBUG_ENTRY* d = &c->debug_entries[i];
        if(n >= d->start && n < d->start + d->len)
            context_name = d->context_name;
    }
    
    return context_name;
}

void compile_call(COMPILER* c, PNODE const* into) {
    ANODE* n = next_anode(c, is_primitive(into) ? ANODE_CALL_PRIMITIVE : ANODE_CALL_FUNC);
    n->data.into = into;
}

void compile_literal(COMPILER* c, VALUE v) {
    ANODE* n = next_anode(c, ANODE_LITERAL);
    n->data.value = v;
}

ANODE* compile_cjump(COMPILER* c) {
    ANODE* n = next_anode(c, ANODE_CJUMP);
    n->data.target = NULL;
    return n;
}

ANODE* compile_jump(COMPILER* c) {
    ANODE* n = next_anode(c, ANODE_JUMP);
    n->data.target = NULL;
    return n;
}

void compile_recur(COMPILER* c) {
    ANODE* n = next_anode(c, ANODE_RECUR);
    n->data.into = NULL;
}

void compile_stub(COMPILER* c) {
    
}

ANODE* compiler_pos(COMPILER* c) {
    return curr_program(c)->spare;
}

void resolve_jump(ANODE* jnode, ANODE* dest) {
    jnode->data.target = dest;
}

int compiler_is_compiling(COMPILER* c) {
    return c->program_sp > c->program_stack;
}
