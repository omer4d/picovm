#ifndef __TYPES_H__
#define __TYPES_H__

#include "map.h"

struct MAP_OBJECT_t;

typedef struct {
    struct MAP_OBJECT_t* proto;
}OBJECT;

typedef struct MAP_OBJECT_t {
    OBJECT base;
    MAP map;
}MAP_OBJECT;

#endif
