#include "tokenizer.h"
#include <ctype.h>
#include <assert.h>

void init_tokenizer(TOKENIZER* tokenizer, char const* str) {
    tokenizer->pos = str;
}

TOK_TYPE next_tok(TOKENIZER* tokenizer, char* tok) {
    char* tok_pos = tok;
    
    start:
        if(*tokenizer->pos == 0)
            return TOK_END;
        else if(isspace(*tokenizer->pos)) {
            ++tokenizer->pos;
            goto start;
        }else if(isdigit(*tokenizer->pos)) {
            *(tok_pos++) = *(tokenizer->pos++);
            goto word_or_num;
        }else {
            assert(isprint(*tokenizer->pos));
            *(tok_pos++) = *(tokenizer->pos++);
            goto word;
        }
    word:
        if(*tokenizer->pos == 0 || isspace(*tokenizer->pos)) {
            *(tok_pos++) = 0;
            return TOK_WORD;
        }else {
            assert(isprint(*tokenizer->pos));
            *(tok_pos++) = *(tokenizer->pos++);
            goto word;
        }
    word_or_num:
        if(*tokenizer->pos == 0 || isspace(*tokenizer->pos)) {
            *(tok_pos++) = 0;
            return TOK_NUM;
        }else if(isdigit(*tokenizer->pos)) {
            *(tok_pos++) = *(tokenizer->pos++);
            goto word_or_num;
        }else {
            assert(isprint(*tokenizer->pos));
            *(tok_pos++) = *(tokenizer->pos++);
            goto word;
        }
}
