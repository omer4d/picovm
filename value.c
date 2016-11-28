#include "value.h"
#include <assert.h>

VALUE num_value(double x) {
    VALUE v;
    v.type = NUM_TYPE;
    v.data.num = x;
    return v;
}

VALUE func_value(void* fn) {
    VALUE v;
    v.type = FUNC_TYPE;
    v.data.func = fn;
    return v;
}

int values_equal(VALUE* a, VALUE* b) {
    if(a->type != b->type)
        return 0;
    else {
        switch(a->type) {
            case BOOL_TYPE:
                return a->data.boolean == b->data.boolean;
            case NUM_TYPE:
                return a->data.num == b->data.num;
            case FUNC_TYPE:
                return a->data.func == b->data.func;
            case OBJECT_TYPE:
                return a->data.obj == b->data.obj;
            default:
                assert(0);
        }
    }
}

unsigned int sax_hash(void *data, int len) {
    unsigned char *p = data;
    unsigned int h = 0;
    int i;

    for (i = 0; i < len; i++) {
        h ^= (h << 5) + (h >> 2) + p[i];
    }

    return h;
}

unsigned int value_hash(VALUE* key) {
    unsigned int h;
    
    switch(key->type) {
        case BOOL_TYPE:
            h = sax_hash(&key->data.boolean, sizeof(int));
            break;
        case NUM_TYPE:
            h = sax_hash(&key->data.num, sizeof(double));
            break;
        case FUNC_TYPE:
            h = sax_hash(&key->data.func, sizeof(void*));
            break;
        case OBJECT_TYPE:
            h = sax_hash(&key->data.num, sizeof(struct OBJECT_BASE_t*));
            break;
        default:
            assert(0);
    }
    
    return h;
}
