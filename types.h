#ifndef __TYPES_H__
#define __TYPES_H__

#include "map.h"

struct OBJECT_t;

typedef struct OBJECT_BASE_t {
    struct OBJECT_t* meta;
}OBJECT_BASE;

typedef struct OBJECT_t {
    OBJECT_BASE base;
    MAP map;
}OBJECT;

OBJECT* create_object();
void destroy_object(OBJECT* obj);
int has_default_meta(OBJECT* obj);

void init_object_system(OBJECT* meta);

extern OBJECT* default_meta;

#endif
