#include "catom.h"
#include "memalloc.h"
#include "vbprint.h"

#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

static PointerListNode CONST_TAIL;
static PointerListNode CONST_HEAD = { NULL, 0, NULL, &CONST_TAIL };
static PointerListNode CONST_TAIL = { NULL, 0, &CONST_HEAD, NULL };

static PointerList list = { &CONST_HEAD, &CONST_TAIL };

void *testfunc_malloc(const size_t bytes) {
    void *mem = malloc(bytes);
    if (mem) {
        PointerListNode *tail = list.tail;
        PointerListNode *prev = tail->pred;

        // Create a new list node to store the newly allocated pointer.
        PointerListNode *node = (PointerListNode *) malloc(sizeof(PointerListNode));
        if (!node) {
            return NULL;
        }

        tail->pred = node;
        prev->succ = node;

        node->pred = prev;
        node->succ = tail;

        // Assign the other info in the node.
        node->ptr = mem;
        node->bytes = bytes;

        vbprintf(stderr, "MEMORY: Allocated %zu bytes of memory at @%" PRIuPTR "!\n", bytes, mem);
        return mem;
    }

    return NULL;
}

void *testfunc_calloc(const size_t n, const size_t size) {
    void *mem = testfunc_malloc(n * size);
    if (mem) {
        memset(mem, 0, n * size);
        vbprintf(stderr, "MEMORY: Zeroed %zu bytes of memory at @%" PRIuPTR "!\n", n * size, mem);

        return mem;
    }

    return NULL;
}

void *testfunc_realloc(void *ptr, const size_t bytes) {
    if (!ptr) {
        return testfunc_malloc(bytes);
    }

    for (PointerListNode *cur = list.head; cur != list.tail; cur = cur->succ) {
        if (cur->ptr == ptr) {
            void *old_ptr = cur->ptr;
            size_t old_bytes = cur->bytes;

            cur->ptr = realloc(cur->ptr, bytes);
            if (!cur->ptr) {
                return NULL;
            }

            cur->bytes = bytes;

            vbprintf(stderr, "MEMORY: Reallocated %zu bytes of memory at @%" PRIuPTR " (from %zu bytes at @%" PRIuPTR ")!\n", bytes, cur->ptr, old_bytes, old_ptr);

            return cur->ptr;
        }
    }

    return NULL;
}

void testfunc_free(void *ptr) {
    if (!ptr) {
        return;
    }

    for (PointerListNode *cur = list.head; cur != list.tail; cur = cur->succ) {
        if (cur->ptr == ptr) {
            free(cur->ptr);
            vbprintf(stderr, "MEMORY: Freed %zu bytes of memory at @%" PRIuPTR "!\n", cur->bytes, cur->ptr);

            cur->pred->succ = cur->succ;
            cur->succ->pred = cur->pred;

            free(cur);
            return;
        }
    }
}

void testfunc_freeall(void) {
    PointerListNode *cur = list.head->succ;

    if (cur != list.tail) {
        vbprintf(stderr, "\n");
    }

    while (cur != list.tail) {
        testfunc_free(cur->ptr);
        cur = cur->succ;
    }
}
