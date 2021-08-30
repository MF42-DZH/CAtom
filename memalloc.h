#ifndef __MEMALLOC_H__
#define __MEMALLOC_H__

#include <stddef.h>

/**
 * A struct representing the nodes in the linked list used for keeping track of the memory allocated.
 */
typedef struct PLN {
    void *ptr;        /**< Pointer to the allocated memory location. */
    size_t bytes;     /**< Number of bytes allocated. */

    struct PLN *pred; /**< Previous node. */
    struct PLN *succ; /**< Next node. */
} PointerListNode;

typedef struct {
    PointerListNode *head; /**< Head of the linked list. */
    PointerListNode *tail; /**< Tail of the linked list. */
} PointerList;

/**
 * Initialise the memory pointer list.
 */
void init_ptr_list(void);

/**
 * Attempt to free every pointer allocated by the testfunc_?alloc functions.
 *
 * WARNING: This function is automatically called after a test finishes.
 */
void testfunc_freeall(void);

#endif  // __MEMALLOC_H__
