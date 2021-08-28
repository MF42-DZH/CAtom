/**
 * This file assists in finding alloca, the function that allocates memory on the
 * stack as opposed to in the heap.
 *
 * alloca - Attempt to allocate memory from the stack of sz bytes. If allocation
 *          is successful, it returns a pointer to the start of the memory
 *          location allocated on the stack. Otherwise, a NULL pointer is
 *          returned. If an allocation results in a stack overflow, program
 *          behaviour is undefined from that point onwards. See man page
 *          alloca(3) or https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/alloca?view=msvc-160
 *          for more information on alloca.
 *
 * void *alloca(size_t sz);
 */

#ifndef __SALLOC_H__
#define __SALLOC_H__

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
// In Windows (MinGW or MSVC, _alloca is in malloc.h).
// alloca exists as a macro for _alloca.
#include <malloc.h>
#else
// In Mac OS / Linux, alloca is in alloca.h.
// alloca exists as a discrete function.
#include <alloca.h>
#endif

#endif
