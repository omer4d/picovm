#ifndef __MAP_H__
#define __MAP_H__

#include "value.h"

typedef struct {
    int capacity;
    struct MAP_NODE_t* data;
    struct MAP_NODE_t* next_free;
}MAP;

void init_map(MAP* m, int capacity);
void cleanup_map(MAP* m);
MAP* create_map(int capacity);
void destroy_map(MAP* m);

void map_put(MAP* m, VALUE const* key, VALUE const* item);
void map_get(VALUE* out, MAP const* m, VALUE const* key);
void map_foreach(MAP* m, void (*func)(VALUE const* key, VALUE* val, void* data), void* data);

void test_map();

#endif
