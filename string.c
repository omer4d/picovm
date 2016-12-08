#include "string.h"

STRING* create_string(char const* data, int len, OBJECT* meta) {
    STRING* str = malloc(sizeof(STRING));
    str->base.meta = meta;
    str->data = data;
    str->len = len;
    return str;
}

void destroy_string(STRING* s) {
    free((void*)s->data);
    free(s);
}
