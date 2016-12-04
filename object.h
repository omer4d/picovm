#ifndef __TYPES_H__
#define __TYPES_H__

#include "map.h"

struct VM_t;
struct OBJECT_t;

typedef struct OBJECT_BASE_t {
    struct OBJECT_t* meta;
}OBJECT_BASE;

typedef struct OBJECT_t {
    OBJECT_BASE base;
    MAP map;
}OBJECT;

OBJECT* create_object(OBJECT* meta);
void destroy_object(OBJECT* obj);

#endif
