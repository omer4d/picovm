#ifndef __TOKENIZER_H__
#define __TOKENIZER_H__

typedef enum {
    TOK_WORD, TOK_NUM, TOK_END
}TOK_TYPE;

typedef struct {
    char const* pos;
}TOKENIZER;

void init_tokenizer(TOKENIZER* tokenizer, char const* str);
TOK_TYPE next_tok(TOKENIZER* tokenizer, char* tok);

#endif
