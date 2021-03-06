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

VALUE string_value(STRING* s) {
    VALUE v = {.type = STRING_TYPE, .data.obj = (OBJECT_BASE*)s};
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
            case NIL_TYPE:
                return 1;
            case BOOL_TYPE:
                return a->data.boolean == b->data.boolean;
            case NUM_TYPE:
                return a->data.num == b->data.num;
            case CREF_TYPE:
                return a->data.cref.ptr == b->data.cref.ptr;
            case FUNC_TYPE: case SYMBOL_TYPE: case OBJECT_TYPE:
                return a->data.obj == b->data.obj;
            case STRING_TYPE: 
                return strings_equal((STRING*)a->data.obj, (STRING*)b->data.obj);
            default:
                assert(0);
        }
    }
}

char* value_to_string(char* str, int n, VALUE* sp) {
    int written;
    switch(sp->type) {
        case BOOL_TYPE:
            written = snprintf(str, n, "%s", sp->data.boolean ? "true" : "false");
            break;
        case NUM_TYPE:
            written = snprintf(str, n, "%f", sp->data.num);
            break;
        case FUNC_TYPE:
            written = snprintf(str, n, "<function %s>", ((FUNC*)sp->data.obj)->name ? ((FUNC*)sp->data.obj)->name : "unknown");
            break;
        case STRING_TYPE:
            written = snprintf(str, n, "\"%s\"", ((STRING*)sp->data.obj)->data);
            break;
        case SYMBOL_TYPE:
            written = snprintf(str, n, "'%s", ((SYMBOL*)sp->data.obj)->name);
            break;
        case NIL_TYPE:
            written = snprintf(str, n, "nil");
            break;
        case OBJECT_TYPE:
            written = snprintf(str, n, "<object>");
            break;
        case CREF_TYPE:
            written = snprintf(str, n, "<cref(%d) %p>", sp->data.cref.tag, sp->data.cref.ptr);
            break;
        default:
            assert(0);
    }
    
    if(written > n - 1 && n > 3)
        str[n - 2] = str[n - 3] = str[n - 4] = '.';
    
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
        case NIL_TYPE:
            h = 0;
        case BOOL_TYPE:
            h = sax_hash(&key->data.boolean, sizeof(int));
            break;
        case NUM_TYPE:
            h = sax_hash(&key->data.num, sizeof(double));
            break;
        case FUNC_TYPE: case SYMBOL_TYPE: case OBJECT_TYPE:
            h = sax_hash(&key->data.obj, sizeof(struct OBJECT_BASE_t*));
            break;
        case STRING_TYPE:
            h = sax_hash(((STRING*)key->data.obj)->data, ((STRING*)key->data.obj)->len);
            break;
        default:
            assert(0);
    }
    
    return h;
}

int value_is_nil(VALUE const* v) {
    return v->type == NIL_TYPE;
}
