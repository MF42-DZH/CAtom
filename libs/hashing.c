#include "hashing.h"

uint64_t obj_hash(const void *obj, const size_t total_length) {
    uint64_t result = 0u;

    if (obj) {
        result = 1u;
        uint64_t multiplier = 1u;

        for (size_t i = 0; i < total_length; ++i) {
            uint8_t cur = ((uint8_t *) obj)[i];
            result += ((uint64_t) cur) * multiplier;

            multiplier *= HASH_CONSTANT;
        }
    }

    return result;
}
