#include "types.h"
#include <assert.h>

int values_equal(VALUE* a, VALUE* b) {
    if(a->type != b->type)
        return 0;
    else {
        switch(a->type) {
            case BOOL_TYPE:
                return (a->data.boolean && b->data.boolean) || !(a->data.boolean || b->data.boolean);
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

