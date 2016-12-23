#include "tokenizer.h"
#include <ctype.h>
#include <assert.h>

#include "tokenizer.h"
#include "charstream.h"
#include <ctype.h>
#include <assert.h>

TOK_TYPE next_tok(char* tok, CHARSTREAM* stream) {
    char* tok_pos = tok;
    char c;
    
    start:
        c = chs_getc(stream);
        if(c == EOF || c == '\n')
            return TOK_END;
        else if(isspace(c)) {
            goto start;
        }else if(isdigit(c)) {
            *(tok_pos++) = c;
            goto integer_part;
        }else if(c == '-' || c == '+') {
            *(tok_pos++) = c;
            goto signed_num;
        }else {
            assert(isprint(c));
            *(tok_pos++) = c;
            goto word;
        }
    word:
        c = chs_getc(stream);
        if(c == EOF || isspace(c)) {
            chs_ungetc(c, stream);
            *(tok_pos++) = 0;
            return TOK_WORD;
        }else {
            assert(isprint(c));
            *(tok_pos++) = c;
            goto word;
        }
    signed_num:
        c = chs_getc(stream);
        if(c == EOF || isspace(c)) {
            chs_ungetc(c, stream);
            *(tok_pos++) = 0;
            return TOK_WORD;
        }else if(isdigit(c)) {
            *(tok_pos++) = c;
            goto integer_part;
        }else if(c == '.') {
            *(tok_pos++) = c;
            goto fractional_part;
        }else {
            assert(isprint(c));
            *(tok_pos++) = c;
            goto word;
        }
    integer_part:
        c = chs_getc(stream);
        if(c == EOF || isspace(c)) {
            chs_ungetc(c, stream);
            *(tok_pos++) = 0;
            return TOK_NUM;
        }else if(isdigit(c)) {
            *(tok_pos++) = c;
            goto integer_part;
        }else if(c == '.') {
            *(tok_pos++) = c;
            goto fractional_part;
        }else {
            assert(isprint(c));
            *(tok_pos++) = c;
            goto word;
        }
    fractional_part:
        c = chs_getc(stream);
        if(c == EOF || isspace(c)) {
            chs_ungetc(c, stream);
            *(tok_pos++) = 0;
            return TOK_NUM;
        }else if(isdigit(c)) {
            *(tok_pos++) = c;
            goto fractional_part;
        }else {
            assert(isprint(c));
            *(tok_pos++) = c;
            goto word;
        }
}
