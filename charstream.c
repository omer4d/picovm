#include "charstream.h"
#include <assert.h>

int chs_getc(CHARSTREAM* ls) {
    if(ls->pos >= 512 || !ls->buff[ls->pos]) {
        if(fgets(&ls->buff[1], 511, ls->file)) {
            ls->pos = 1;
        }else {
            return EOF;
        }
    }
    
    return ls->buff[ls->pos++];
}

void chs_drop(CHARSTREAM* ls) {
    ls->pos = 1;
    ls->buff[1] = 0;
}

int chs_ungetc(CHARSTREAM* ls, int c) {
    if(c != EOF) {
        assert(ls->pos > 0 && "Must read at least one character before calling ls_ungetc again!");
        ls->buff[--ls->pos] = c;
    }
    return c;
}