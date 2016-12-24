#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "map.h"

typedef struct MAP_NODE_t {
    VALUE key;
    VALUE item;
    struct MAP_NODE_t* prev;
    struct MAP_NODE_t* next;
}NODE;

NODE* const sentinel = &(NODE){PVM_NIL_INIT, PVM_NIL_INIT, NULL, NULL};

NODE* create_map_data(int capacity) {
    NODE* data = (NODE*)malloc(sizeof(NODE) * capacity);
    
    data[0].key = PVM_NIL;
    data[0].item = PVM_NIL;
    data[0].prev = sentinel;
    
    int i;
    for(i = 1; i < capacity; ++i) {
        data[i].key = PVM_NIL;
        data[i].item = PVM_NIL;
        data[i].prev = &data[i - 1];
        data[i - 1].next = &data[i];
    }
    
    data[capacity - 1].next = sentinel;
    
    return data;
}

void init_map(MAP* m, int capacity) {
    m->data = m->next_free = create_map_data(capacity);
    m->capacity = capacity;
}

void cleanup_map(MAP* m) {
    free(m->data);
}

MAP* create_map(int capacity) {
    MAP* m = malloc(sizeof(MAP));
    init_map(m, capacity);
    return m;
}

void destroy_map(MAP* m) {
    cleanup_map(m);
    free(m);
}

int mod(int a, int b) {
    int m = a % b;
    return m < 0 ? m + b : m;
}

void map_put(MAP* m, VALUE const* key, VALUE const* item);

void expand(MAP* m) {
    NODE* old_data = m->data;
    int old_capacity = m->capacity;
    m->capacity = m->capacity * 2;
    m->data = create_map_data(m->capacity);
    m->next_free = &m->data[0];
  
    int i;
    for(i = 0; i < old_capacity; ++i)
        map_put(m, &old_data[i].key, &old_data[i].item);
    
    free(old_data);
}

NODE* get_free_node(MAP* m) {
    NODE* curr = m->next_free;
    m->next_free = m->next_free->next;
    m->next_free->prev = sentinel;
    return curr;
}

void init_node(NODE* n, VALUE const* key, VALUE const* item, NODE* prev, NODE* next) {
    n->key = *key;
    n->item = *item;
    n->prev = prev;
    n->next = next;
}

void remove_node(NODE* n) {
    n->prev->next = n->next;
    n->next->prev = n->prev;
}

void replace_node(NODE* n0, NODE* n1) {
    n1->prev = n0->prev;
    n1->next = n0->next;
    n0->prev->next = n1;
    n0->next->prev = n1;
}

NODE* own_node(MAP const* m, VALUE const* key) {
    return &m->data[mod(value_hash(key), m->capacity)];
}

int new_bucket, same_bucket, wrong_bucket;

void map_put(MAP* m, VALUE const* key, VALUE const* item) {
    NODE* n = own_node(m, key);
    
    if(n->item.type == OBJECT_TYPE && n->item.data.obj == NULL) {
        ++new_bucket;
        if(n == m->next_free)
            get_free_node(m);
        else
            remove_node(n);
        init_node(n, key, item, sentinel, sentinel);
    }
    
    else {
        if(n->prev == sentinel) {
            for(;;n = n->next) {
                if(values_equal(&n->key, key)) {
                    n->item = *item;
                    return;
                }
                else if(n->next == sentinel)
                    break;
            }
      
        if(m->next_free == sentinel) {
            expand(m);
            map_put(m, key, item);
        }else {
            ++same_bucket;
            NODE* curr = get_free_node(m);
            init_node(curr, key, item, n, sentinel);
            n->next = curr;
        }
      }
      
      else if(m->next_free == sentinel) {
            expand(m);
            map_put(m, key, item);
      }
      
      else {
            ++wrong_bucket;
            NODE* curr = get_free_node(m);
            curr->key = n->key;
            curr->item = n->item;
            replace_node(n, curr);
            init_node(n, key, item, sentinel, sentinel);
      }
    }
}

void map_get(VALUE* out, MAP const* m, VALUE const* key) {
    NODE* n;
    
    for(n = own_node(m, key); n != sentinel; n = n->next) {
        if(values_equal(&n->key, key)) {
            *out = n->item;
            return;
        }
    }
    
    *out = PVM_NIL;
}

void map_foreach(MAP* m, void (*func)(VALUE const* key, VALUE* val, void* data), void* data) {
    for(NODE* n = m->data; n < m->data + m->capacity; ++n) {
        if(!value_is_nil(&n->item)) {
            func(&n->key, &n->item, data);
        }
    }
}

unsigned int gen_key_helper(unsigned int x) {
    static const unsigned int prime = 4294967291u;
    if (x >= prime)
        return x;
    unsigned int residue = ((unsigned long long) x * x) % prime;
    return (x <= prime / 2) ? residue : prime - residue;
}

unsigned int gen_key(unsigned int x) {
    return gen_key_helper(gen_key_helper(x) ^ 0x5bf03635);
}

void test_map() {
    new_bucket = 0;
    same_bucket = 0;
    wrong_bucket = 0;

    MAP* m = create_map(8);
    int i, j;
    
    for(i = 0; i < 1000; ++i) {
        VALUE key;
        key.type = NUM_TYPE;
        key.data.num = gen_key(i);
        
        VALUE item;
        item.type = NUM_TYPE;
        item.data.num = i;
        
        map_put(m, &key, &item);
        
        for(j = 0; j <= i; ++j) {
            VALUE key;
            key.type = NUM_TYPE;
            key.data.num = gen_key(i);
            VALUE item;
            map_get(&item, m, &key);
            if(value_is_nil(&item) || item.data.num != i) {
                printf("WTF!");
                return;
            }
        }
    }
    
    printf("DONE: %d\n", m->capacity);
    printf("unique: %d\ncollisions: %d\ncollisions with swap: %d\n", same_bucket, new_bucket, wrong_bucket);
}

