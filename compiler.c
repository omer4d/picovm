#include "compiler.h"
#include "macros.h"
#include "object.h"
#include "tokenizer.h"
#include "primitives.h"

PROGRAM_WRITER* curr_pw(COMPILER* c) {
    assert(c->pw_sp > c->pw_stack);
    return c->pw_sp - 1;
}


void init_compiler(COMPILER* c) {
    c->in = stdin;
    c->pw_stack = malloc(sizeof(PROGRAM_WRITER) * PW_STACK_SIZE);
    c->pw_sp = c->pw_stack;
}

void cleanup_compiler(COMPILER* c) {
    PROGRAM_WRITER* pw;
    for(pw = c->pw_stack; pw < c->pw_sp; ++pw)
        cleanup_program_writer(pw);
    free(c->pw_stack);
}

void begin_compilation(COMPILER* c) {
    ASSERT_PUSH(c->pw_stack, c->pw_sp, PW_STACK_SIZE);
    init_program_writer(c->pw_sp++);
}

int is_special_primitive(PNODE const* pnode) {
    return pnode == &primitives[jump_loc] ||
            pnode == &primitives[cjump_loc] ||
            pnode == &primitives[push_loc];
}

int is_primitive(PNODE const* pnode) {
    return pnode >= primitives && pnode < primitives + PRIMITIVE_NUM;
}

PNODE* perform_tco(PROGRAM prog) {
    // Assumption: program either ends with 'leave' or (in case of a stub) 'jump'
    /*
    int i = 0;    
    while(i < prog.len - 1) {
        PNODE const* into = prog.data[i].into;
        if(is_special_primitive(into))
            i += 2;
        else {
            if(!is_primitive(into) && prog.data[i + 1].into == &primitives[leave_loc]) {
                printf("WTF!");
                prog.data[i].into = &primitives[jump_loc];
                prog.data[i + 1].into = into + 1;
            }
            ++i;
        }  
    }*/   
    
    return prog.data;
}

PNODE* end_compilation(COMPILER* c) {
    ASSERT_POP(c->pw_stack, c->pw_sp);
    PROGRAM prog = acquire_program(curr_pw(c));
    cleanup_program_writer(--c->pw_sp);
    return perform_tco(prog);
}

void compile_func_enter(COMPILER* c) {
    write_cfun(curr_pw(c), primitives[enter_loc].fp);
}

void compile_call(COMPILER* c, PNODE const* into) {
    write_pnode(curr_pw(c), into);
}

void compile_literal(COMPILER* c, VALUE v) {
    write_pnode(curr_pw(c), &primitives[push_loc]);
    write_value(curr_pw(c), v);
}

void compile_cjump(COMPILER* c) {
    write_pnode(curr_pw(c), &primitives[cjump_loc]);
    program_writer_mark(curr_pw(c));
}

void compile_jump(COMPILER* c) {
    write_pnode(curr_pw(c), &primitives[jump_loc]);
    program_writer_mark(curr_pw(c));
}

void compile_recur(COMPILER* c) {
    write_pnode(curr_pw(c), &primitives[jump_loc]);
    write_rel_addr(curr_pw(c), 1);
    //program_writer_mark(curr_pw(c));
}

void compile_stub(COMPILER* c) {
    
}

void compiler_resolve(COMPILER* c, int mark_id) {
    program_writer_resolve(curr_pw(c), mark_id);
}

void compiler_drop_marks(COMPILER* c, int n) {
    program_writer_drop_marks(curr_pw(c), n);
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
//VALUE parse_num(char const* str) {
//    VALUE v = {.type = NUM_TYPE, .data.num = strtod(str, NULL)};
//    return v;
//}
//
//VALUE parse_word(COMPILER* c, char const* str) {
//    VALUE v = {.type = SYMBOL_TYPE, .data.obj = (OBJECT_BASE*)get_symbol(c, str)};
//    return v;
//}
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

