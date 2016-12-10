#include <assert.h>

#define ASSERT_PUSH(start, sp, size) assert((sp) < (start) + (size))
#define ASSERT_POP(start, sp) assert((sp) > (start))
