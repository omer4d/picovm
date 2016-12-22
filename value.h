#ifndef __VALUE_H__
#define __VALUE_H__

#include "cref.h"

#define PVM_NIL_INIT { .type = OBJECT_TYPE, .data.obj = ((void*)0) }
#define PVM_NIL (VALUE)PVM_NIL_INIT

#define PVM_TRUE_INIT { .type = BOOL_TYPE, .data.boolean = 1 }
#define PVM_TRUE (VALUE)PVM_TRUE_INIT

#define PVM_FALSE_INIT { .type = BOOL_TYPE, .data.boolean = 0 }
#define PVM_FALSE (VALUE)PVM_FALSE_INIT

struct OBJECT_BASE_t;

typedef enum {
    BOOL_TYPE, NUM_TYPE, FUNC_TYPE, STRING_TYPE, SYMBOL_TYPE, OBJECT_TYPE, CREF_TYPE
}VALUE_TYPE;

typedef struct {
    VALUE_TYPE type;
    union {
        int boolean; // only 0 and 1 are valid values
        double num;
        struct OBJECT_BASE_t* obj;
        CREF cref;
    }data;
}VALUE;

VALUE num_value(double x);
VALUE cref_value(void* ptr, int tag);
int values_equal(VALUE const* a, VALUE const* b); // returns 0 or 1
char* value_to_string(char* str, VALUE* sp);
unsigned int value_hash(VALUE const* v);
int value_is_nil(VALUE const* v);

#endif
