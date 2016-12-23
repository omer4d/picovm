#include "charstream.h"
#include <assert.h>
#include <malloc.h>

typedef struct {
    int (*getc)(CHARSTREAM* ls);
    int (*ungetc)(int c, CHARSTREAM* ls);
    void (*drop)(CHARSTREAM* ls);
}CHARSTREAM_OPERATIONS;

typedef struct CHARSTREAM_t {
    FILE* file;
    char buff[512];
    int pos;
    //CHARSTREAM_OPERATIONS* operations;
}CHARSTREAM;

CHARSTREAM* create_file_charstream(FILE* file) {
    CHARSTREAM* chs = malloc(sizeof(CHARSTREAM));
    chs->file = file;
    chs->buff[1] = 0;
    chs->pos = 1;
    return chs;
}

void destroy_charstream(CHARSTREAM* chs) {
    free(chs);
}

int chs_getc(CHARSTREAM* chs) {
    if(chs->pos >= 512 || !chs->buff[chs->pos]) {
        if(fgets(&chs->buff[1], 511, chs->file)) {
            chs->pos = 1;
        }else {
            return EOF;
        }
    }
    
    return chs->buff[chs->pos++];
}

void chs_drop(CHARSTREAM* chs) {
    chs->pos = 1;
    chs->buff[1] = 0;
}

int chs_ungetc(int c, CHARSTREAM* chs) {
    if(c != EOF) {
        assert(chs->pos > 0 && "Must read at least one character before calling ls_ungetc again!");
        chs->buff[--chs->pos] = c;
    }
    return c;
}

int chs_eof(CHARSTREAM* chs) {
    return feof(chs->file);
}