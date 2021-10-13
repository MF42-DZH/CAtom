#include "genarrays.h"

#include <stdint.h>

static const void *get_from_flat(const void *arr, const size_t size, const size_t ns[], const size_t argn, const size_t *where) {
    // Assuming C arrays are in row-major order...
    size_t offset = 0;
    size_t mult = 1;
    for (size_t n = argn - 1; n < argn; --n) {
        offset += where[n] * mult;
        mult *= ns[n];
    }

    return (uint8_t *) arr + (offset * size);
}

static const void *get_from_ptp(const void *arr, const size_t size, const size_t argn, const size_t *where) {
    if (argn == 1) {
        return (uint8_t *) arr + (where[0] * size);
    } else {
        return get_from_ptp(*((uint8_t **) arr + where[0]), size, argn - 1, where + 1);
    }
}

const void *get(const void *arr, const bool isptp, const size_t size, const size_t ns[], const size_t argn, const size_t *where) {
    if (isptp) {
        return get_from_ptp(arr, size, argn, where);
    } else {
        return get_from_flat(arr, size, ns, argn, where);
    }
}
