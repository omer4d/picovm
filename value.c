#include "value.h"
#include "func.h"
#include "string.h"
#include "symbol.h"
#include <assert.h>
#include <stdio.h>

VALUE num_value(double x) {
    VALUE v;
    v.type = NUM_TYPE;
    v.data.num = x;
    return v;
}

VALUE cref_value(void* ptr, int tag) {
    VALUE v;
    v.type = CREF_TYPE;
    v.data.cref.ptr = ptr;
    v.data.cref.tag = tag;
    return v;
}

int values_equal(VALUE const* a, VALUE const* b) {
    if(a->type != b->type) {
        return 0;
    }
    else {
        switch(a->type) {
            case BOOL_TYPE:
                return a->data.boolean == b->data.boolean;
            case NUM_TYPE:
                return a->data.num == b->data.num;
            case CREF_TYPE:
                return a->data.cref.ptr == b->data.cref.ptr;
            case FUNC_TYPE: case STRING_TYPE: case SYMBOL_TYPE: case OBJECT_TYPE:
                return a->data.obj == b->data.obj;
            default:
                assert(0);
        }
    }
}

char* value_to_string(char* str, VALUE* sp) {
    switch(sp->type) {
        case BOOL_TYPE:
            sprintf(str, "%s", sp->data.boolean ? "true" : "false");
            break;
        case NUM_TYPE:
            sprintf(str, "%f", sp->data.num);
            break;
        case FUNC_TYPE:
            sprintf(str, "<function %s>", ((FUNC*)sp->data.obj)->name ? ((FUNC*)sp->data.obj)->name : "unknown");
            break;
        case STRING_TYPE:
            sprintf(str, "\"%s\"", ((STRING*)sp->data.obj)->data);
            break;
        case SYMBOL_TYPE:
            sprintf(str, "'%s", ((SYMBOL*)sp->data.obj)->name);
            break;
        case OBJECT_TYPE:
            sprintf(str, value_is_nil(sp) ? "nil" : "<object>");
            break;
        case CREF_TYPE:
            sprintf(str, "<cref(%d) %x>", sp->data.cref.tag, sp->data.cref.ptr);
            break;
        default:
            assert(0);
    }
    
    return str;
}

unsigned int sax_hash(void const* data, int len) {
    unsigned char const* p = data;
    unsigned int h = 0;
    int i;

    for (i = 0; i < len; i++) {
        h ^= (h << 5) + (h >> 2) + p[i];
    }

    return h;
}

unsigned int value_hash(VALUE const* key) {
    unsigned int h;
    
    switch(key->type) {
        case BOOL_TYPE:
            h = sax_hash(&key->data.boolean, sizeof(int));
            break;
        case NUM_TYPE:
            h = sax_hash(&key->data.num, sizeof(double));
            break;
        case FUNC_TYPE: case STRING_TYPE: case SYMBOL_TYPE: case OBJECT_TYPE:
            h = sax_hash(&key->data.obj, sizeof(struct OBJECT_BASE_t*));
            break;
        default:
            assert(0);
    }
    
    return h;
}

int value_is_nil(VALUE const* v) {
    return v->type == OBJECT_TYPE && !v->data.obj;
}
