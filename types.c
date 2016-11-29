#include "types.h"

OBJECT* default_meta = (OBJECT*)0;

void init_object_system(OBJECT* meta) {
    default_meta = meta;
}

OBJECT* create_object() {
    OBJECT* obj = malloc(sizeof(OBJECT));
    obj->base.meta = default_meta;
    init_map(&obj->map, 8);
    return obj;
}

void destroy_object(OBJECT* obj) {
    cleanup_map(&obj->map);
    free(obj);
}

int has_default_meta(OBJECT* obj) {
    return obj->base.meta == default_meta;
}
