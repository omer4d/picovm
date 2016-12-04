#include "object.h"

OBJECT* create_object(OBJECT* meta) {
    OBJECT* obj = malloc(sizeof(OBJECT));
    obj->base.meta = meta;
    init_map(&obj->map, 8);
    return obj;
}

void destroy_object(OBJECT* obj) {
    cleanup_map(&obj->map);
    free(obj);
}
