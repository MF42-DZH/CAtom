#ifndef __HASHING_H__
#define __HASHING_H__

#include <stddef.h>
#include <stdint.h>

// Constant used for the hashing process.
#define HASH_CONSTANT ((uint64_t) 524287ul)

// Get the 64-bit hash of an object.
uint64_t obj_hash(const void *obj, const size_t total_length);

#endif  // __HASHING_H__
