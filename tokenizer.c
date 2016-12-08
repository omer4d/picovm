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
            goto integer_part;
        }else if(*tokenizer->pos == '-' || *tokenizer->pos == '+') {
            *(tok_pos++) = *(tokenizer->pos++);
            goto signed_num;
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
    signed_num:
        if(*tokenizer->pos == 0 || isspace(*tokenizer->pos)) {
            *(tok_pos++) = 0;
            return TOK_WORD;
        }else if(isdigit(*tokenizer->pos)) {
            *(tok_pos++) = *(tokenizer->pos++);
            goto integer_part;
        }else if(*tokenizer->pos == '.') {
            *(tok_pos++) = *(tokenizer->pos++);
            goto fractional_part;
        }else {
            assert(isprint(*tokenizer->pos));
            *(tok_pos++) = *(tokenizer->pos++);
            goto word;
        }
    integer_part:
        if(*tokenizer->pos == 0 || isspace(*tokenizer->pos)) {
            *(tok_pos++) = 0;
            return TOK_NUM;
        }else if(isdigit(*tokenizer->pos)) {
            *(tok_pos++) = *(tokenizer->pos++);
            goto integer_part;
        }else if(*tokenizer->pos == '.') {
            *(tok_pos++) = *(tokenizer->pos++);
            goto fractional_part;
        }else {
            assert(isprint(*tokenizer->pos));
            *(tok_pos++) = *(tokenizer->pos++);
            goto word;
        }
    fractional_part:
        if(*tokenizer->pos == 0 || isspace(*tokenizer->pos)) {
            *(tok_pos++) = 0;
            return TOK_NUM;
        }else if(isdigit(*tokenizer->pos)) {
            *(tok_pos++) = *(tokenizer->pos++);
            goto fractional_part;
        }else {
            assert(isprint(*tokenizer->pos));
            *(tok_pos++) = *(tokenizer->pos++);
            goto word;
        }
}
