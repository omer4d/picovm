#ifndef __TYPES_H__
#define __TYPES_H__

#define PVM_NIL_INIT { .type = OBJECT_TYPE, .data.obj = ((void*)0) }
#define PVM_NIL (VALUE)PVM_NIL_INIT

typedef struct MAP_t MAP;

typedef enum {
    BOOL_TYPE, NUM_TYPE, FUNC_TYPE, OBJECT_TYPE
}VALUE_TYPE;

typedef struct OBJECT_t {
    struct OBJECT_t* proto;
}OBJECT;

typedef struct {
    VALUE_TYPE type;
    union {
        int boolean;
        double num;
        void* func;
        OBJECT* obj;
    }data;
}VALUE;

int values_equal(VALUE* a, VALUE* b);

#endif
