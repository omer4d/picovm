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
    c->label_sp = c->label_stack;
    c->mark_sp = c->mark_stack;
    c->program_sp = c->program_stack;
    
    c->debug_entry_num = 0;
}

void cleanup_compiler(COMPILER* c) {
    PROGRAM* p;
    for(p = c->program_stack; p < c->program_sp; ++p)
        cleanup_program(p);
    free(c->program_stack);
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

PNODE* end_compilation(COMPILER* c, char const* context_name) {
    ASSERT_POP(c->program_stack, c->program_sp);
    PROGRAM* prog = curr_program(c);
    
    perform_tco(prog);
    
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
    
    cleanup_program(--c->program_sp);
    return out;
}

char const* find_compilation_context(COMPILER* c, PNODE const* n) {
    int i;
    for(i = 0; i < c->debug_entry_num; ++i) {
        DEBUG_ENTRY* d = &c->debug_entries[i];
        if(n >= d->start && n < d->start + d->len)
            return d->context_name;
    }
    
    return "unknown";
}

void compile_call(COMPILER* c, PNODE const* into) {
    ANODE* n = next_anode(c, is_primitive(into) ? ANODE_CALL_PRIMITIVE : ANODE_CALL_FUNC);
    n->data.into = into;
}

void compile_literal(COMPILER* c, VALUE v) {
    ANODE* n = next_anode(c, ANODE_LITERAL);
    n->data.value = v;
}

void compile_generic_jump(COMPILER* c, ANODE_TYPE type) {
    ANODE* n = next_anode(c, type);
    
    ASSERT_PUSH(c->mark_stack, c->mark_sp, MARK_STACK_SIZE);
    
    n->data.target = c->mark_sp;
    c->mark_sp->data.target = n;
    ++c->mark_sp;
}

void compile_cjump(COMPILER* c) {
    compile_generic_jump(c, ANODE_CJUMP);
}

void compile_jump(COMPILER* c) {
    compile_generic_jump(c, ANODE_JUMP);
}

void compile_recur(COMPILER* c) {
    ANODE* n = next_anode(c, ANODE_RECUR);
    n->data.into = NULL;
}

void compile_stub(COMPILER* c) {
    
}

void compiler_push_label(COMPILER* c) {
    ASSERT_PUSH(c->label_stack, c->label_sp, LABEL_STACK_SIZE);
    *c->label_sp = curr_program(c)->spare;
    ++c->label_sp;
}

void compiler_drop_labels(COMPILER* c, int n) {
    assert(c->label_sp - n >= c->label_stack);
    c->label_sp -= n;
}


void compiler_resolve_generic(COMPILER* c, int mark_id, ANODE* target) {
    assert(mark_id < 0);
    assert(c->mark_sp + mark_id >= c->mark_stack);
    assert(curr_program(c)->size > 0);
    assert((c->mark_sp + mark_id)->data.target); // protect against multiple resolve attempts
      
    (c->mark_sp + mark_id)->data.target->data.target = target;
    (c->mark_sp + mark_id)->data.target = NULL; // Invalidate to prevent multiple resolve attempts
}

void compiler_resolve(COMPILER* c, int mark_id) {
    compiler_resolve_generic(c, mark_id, curr_program(c)->spare);
}

void compiler_resolve_to_label(COMPILER* c, int label_id) {
    assert(label_id < 0);
    assert(c->label_sp + label_id >= c->label_stack);
    compiler_resolve_generic(c, -1, *(c->label_sp + label_id));
    compiler_drop_marks(c, 1);
}

void compiler_drop_marks(COMPILER* c, int n) {
    assert(c->mark_sp - n >= c->mark_stack);
    c->mark_sp -= n;
}

//PNODE* ncons(COMPILER* c, PNODE const* n) {
//    PNODE* node = next_pnode(curr_pw(c));
//    node->into = n;
//    return node;
//}
//
//PNODE* fcons(COMPILER* c, CFUN fp) {
//    PNODE* node = next_pnode(curr_pw(c));
//    node->fp = fp;
//    return node;
//}
//



//
//VALUE program_read(COMPILER* c) {
//    char tok[256];
//    TOK_TYPE tt;
//    
//    for(tt = next_tok(c->in, tok); !feof(c->in); tt = next_tok(c->in, tok)) {
//        switch(tt) {
//            case TOK_WORD:
//                return parse_word(c, tok);
//            case TOK_NUM:
//                return parse_num(tok);
//            default:
//                break;
//        }
//    }
//    
//    assert(!"unexpected eof");
//}
//
//void compile_literal_helper(COMPILER* c, VALUE val) {
//    ncons(c, &primitives[push_loc]);
//    PNODE* tmp = ncons(c, NULL);
//    tmp->value = val;
//}

//void eval_str(COMPILER* c) {
//    char tok[256];
//    TOK_TYPE tt;
//    VALUE key, item;
//    PNODE program_data[1024];
//    PROGRAM prog;
//    init_program(&prog, program_data);
//    
//    c->curr_pw = &prog;
//    PNODE* run = ((FUNC*)lookup(c, "run").data.obj)->pnode;
//    /*PNODE* func_start = *///fcons(c, NULL);
//    
//    for(tt = next_tok(c->in, tok); tt != TOK_END; tt = next_tok(c->in, tok)) {
//        //printf("%s ", tok);
//        
//        switch(tt) {
//            case TOK_WORD:
//                key = symbol_value(c, tok);
//                map_get(&item, &c->global_scope->map, &key);
//                
//                if(value_is_nil(&item)) {
//                    printf("Undefined function '%s'.\n", tok);
//                }else if(item.type != FUNC_TYPE) {
//                    printf("'%s' is not a function.\n", tok);
//                }else {
//                    FUNC* f = (FUNC*)item.data.obj;
//                    if(f->is_macro) {
//                        push(c, item);
//                        c->curr = run;
//                        next(c);
//                        loop(c);
//                    }
//                    else {
//                        ncons(c, f->pnode);
//                    }
//                    /*
//                    push(c, item);
//                    c->curr = run;
//                    next(c);
//                    loop(c);*/
//                    
//                }
//                break;
//            case TOK_NUM:
//                //push(c, parse_num(tok));
//                compile_literal_helper(c, parse_num(tok));
//                break;
//        }
//        
//        
//    }
//    
//    printf("end!");
//    PNODE* exit = ((FUNC*)lookup(c, "exit").data.obj)->pnode;
//    ncons(c, exit);
//    c->curr = program_data - 1;
//    //program_rewind(c);
//    next(c);
//    loop(c);
//    
//    
//    print_debug_info(c);
//}

