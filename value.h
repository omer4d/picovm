#ifndef __VALUE_H__
#define __VALUE_H__

#define PVM_NIL_INIT { .type = OBJECT_TYPE, .data.obj = ((void*)0) }
#define PVM_NIL (VALUE)PVM_NIL_INIT

struct OBJECT_t;

typedef enum {
    BOOL_TYPE, NUM_TYPE, FUNC_TYPE, OBJECT_TYPE
}VALUE_TYPE;

typedef struct {
    VALUE_TYPE type;
    union {
        int boolean; // only 0 and 1 are valid values
        double num;
        void* func;
        struct OBJECT_t* obj;
    }data;
}VALUE;

int values_equal(VALUE* a, VALUE* b);
unsigned int value_hash(VALUE* v);

#endif
