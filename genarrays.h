#ifndef __GENARRAYS_H__
#define __GENARRAYS_H__

#include <stdbool.h>
#include <stddef.h>

// A generalised array access function.
const void *get(const void *arr, const bool isptp, const size_t size, const size_t ns[], const size_t argn, const size_t *where);

#endif  // __GENARRAYS_H__
