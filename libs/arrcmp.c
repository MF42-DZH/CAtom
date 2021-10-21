#include "arrcmp.h"

#include <stdio.h>
#include <string.h>
#include <wchar.h>

bool memory_is_equals(const void *m1, const void *m2, const size_t n) {
    return memcmp(m1, m2, n) == 0;
}

bool memory_is_not_equals(const void *m1, const void *m2, const size_t n) {
    return !memory_is_equals(m1, m2, n);
}
