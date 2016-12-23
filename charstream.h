#ifndef __CHARSTREAM_H__
#define __CHARSTREAM_H__

#include <stdio.h>

typedef struct CHARSTREAM_t CHARSTREAM;

CHARSTREAM* create_file_charstream(FILE* file);
void destroy_charstream(CHARSTREAM* chs);

int chs_getc(CHARSTREAM* chs);
int chs_ungetc(int c, CHARSTREAM* chs);
void chs_drop(CHARSTREAM* chs);
int chs_eof(CHARSTREAM* chs);

#endif

