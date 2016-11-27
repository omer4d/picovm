#ifndef __MAP_H__
#define __MAP_H__

typedef struct {
    int capacity;
    struct MAP_NODE_t* data;
    struct MAP_NODE_t* next_free;
}MAP;

void test_map();

#endif
