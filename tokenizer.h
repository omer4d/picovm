#ifndef __TOKENIZER_H__
#define __TOKENIZER_H__

#include "charstream.h"

typedef enum {
    TOK_WORD, TOK_NUM, TOK_END
}TOK_TYPE;

TOK_TYPE next_tok(char* tok, CHARSTREAM* stream);

#endif
