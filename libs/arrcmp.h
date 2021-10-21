#ifndef __ARRCMP_H__
#define __ARRCMP_H__

#include <stdbool.h>
#include <stddef.h>

// Something that compares two pieces of memory together of a certain size.
typedef bool (*MemoryValidator)(const void *, const void *, const size_t);

bool memory_is_equals(const void *m1, const void *m2, const size_t n);

bool memory_is_not_equals(const void *m1, const void *m2, const size_t n);

#endif  // __ARRCMP_H__
