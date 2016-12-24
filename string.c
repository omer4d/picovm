#include "string.h"

#include <stdlib.h>
#include <string.h>

void init_string(STRING* str, char const* data, int len, OBJECT* meta) {
    str->base.meta = meta;
    str->data = data;
    str->len = len;
}

STRING* create_string(char const* data, int len, OBJECT* meta) {
    STRING* str = malloc(sizeof(STRING));
    init_string(str, data, len, meta);
    return str;
}

void destroy_string(STRING* s) {
    free((void*)s->data);
    free(s);
}

int strings_equal(STRING const* a, STRING const* b) {
    return a == b || (a->len == b->len && !memcmp(a->data, b->data, a->len));
}