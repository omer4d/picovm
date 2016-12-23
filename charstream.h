#ifndef __CHARSTREAM_H__
#define __CHARSTREAM_H__

#include <stdio.h>

typedef struct {
    FILE* file;
    char buff[512];
    int pos;
}CHARSTREAM;

#endif

