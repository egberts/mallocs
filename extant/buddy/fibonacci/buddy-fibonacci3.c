/*
 * Fibonacci Buddy Allocator
 *
 * Fibonacci classes are:
 *
 *     BASE, 2*BASE, 3*BASE, 5*BASE, 8*BASE, ...
 *
 * where:
 *
 *     F[0] = BASE
 *     F[1] = 2 * BASE
 *     F[n] = F[n-1] + F[n-2]
 *
 * A block of class n splits into:
 *
 *     F[n-1] + F[n-2]
 *
 * The lower-address block is F[n-1].
 *
 * The allocator operates on a caller-provided arena.
 *
 * Two buddy lookup implementations are supported:
 *
 *     CONFIG_MALLOC_BUDDY_FIBONACCI_SEARCH
 *         Wave 1. Buddy lookup searches the free lists.
 *
 *     CONFIG_MALLOC_BUDDY_FIBONACCI_LAYOUT
 *         Production. Buddy relationships are maintained explicitly.
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "config.h"
#include "buddy-fibonacci3.h"


/*
 * -------------------------------------------------------------------------
 * Fibonacci size table
 * -------------------------------------------------------------------------
 */

size_t fib_size[FIB_CLASSES];


void
fib_init_sizes(void)
{
    fib_size[0] = FIB_BASE;
    fib_size[1] = 2 * FIB_BASE;

    for (unsigned int i = 2; i < FIB_CLASSES; ++i)
        fib_size[i] = fib_size[i - 1] + fib_size[i - 2];
}


/*
 * The caller-provided arena must contain one maximum-class Fibonacci block.
 */

size_t
fib_arena_size(void)
{
    fib_init_sizes();

    return fib_size[FIB_CLASSES - 1];
}


/*
 * -------------------------------------------------------------------------
 * Allocator state
 * -------------------------------------------------------------------------
 *
 * The arena belongs to the caller. The allocator only retains its address.
 */

unsigned char *fib_heap;
size_t fib_heap_size;
bool fib_initialized;


/*
 * -------------------------------------------------------------------------
 * Free lists
 * -------------------------------------------------------------------------
 */

struct fib_block *fib_free_list[FIB_CLASSES];


void
fib_list_insert(unsigned int order, struct fib_block *block)
{
    block->prev = NULL;
    block->next = fib_free_list[order];

    if (block->next)
        block->next->prev = block;

    fib_free_list[order] = block;
    block->free = true;
}


void
fib_list_remove(unsigned int order, struct fib_block *block)
{
    if (block->prev)
        block->prev->next = block->next;
    else
        fib_free_list[order] = block->next;

    if (block->next)
        block->next->prev = block->prev;

    block->next = NULL;
    block->prev = NULL;
    block->free = false;
}


struct fib_block *
fib_list_take(unsigned int order)
{
    struct fib_block *block = fib_free_list[order];

    if (block)
        fib_list_remove(order, block);

    return block;
}


/*
 * -------------------------------------------------------------------------
 * Size -> Fibonacci class
 * -------------------------------------------------------------------------
 */

unsigned int
fib_order_for_size(size_t size)
{
    for (unsigned int order = 0; order < FIB_CLASSES; ++order) {
        if (fib_size[order] >= size)
            return order;
    }

    return FIB_CLASSES;
}


/*
 * -------------------------------------------------------------------------
 * Wave 1 buddy lookup
 * -------------------------------------------------------------------------
 */

#if defined(CONFIG_MALLOC_BUDDY_FIBONACCI_SEARCH)

struct fib_block *
fib_find_free(unsigned int order, uintptr_t address)
{
    for (struct fib_block *block = fib_free_list[order];
         block;
         block = block->next) {

        if ((uintptr_t)block == address)
            return block;
    }

    return NULL;
}


/*
 * Fibonacci buddy lookup.
 *
 * If block is the larger/lower child:
 *
 *     F[n]
 *      |
 *      +-- F[n-1]   <-- block
 *      |
 *      +-- F[n-2]   <-- buddy
 *
 * If block is the smaller/upper child, the reverse relationship applies.
 *
 * The free-list search verifies that the candidate sibling actually exists
 * as a free block.
 */
struct fib_block *
fib_find_buddy(struct fib_block *block)
{
    unsigned int order = block->order;

    /*
     * Candidate as F[n-1], with F[n-2] immediately following it.
     */
    if (order >= 1) {
        unsigned int buddy_order = order - 1;

        uintptr_t address =
            (uintptr_t)block + fib_size[order];

        struct fib_block *buddy =
            fib_find_free(buddy_order, address);

        if (buddy)
            return buddy;
    }

    /*
     * Candidate as F[n-2], with F[n-1] immediately preceding it.
     */
    if (order + 1 < FIB_CLASSES) {
        unsigned int buddy_order = order + 1;

        uintptr_t address =
            (uintptr_t)block - fib_size[buddy_order];

        struct fib_block *buddy =
            fib_find_free(buddy_order, address);

        if (buddy)
            return buddy;
    }

    return NULL;
}

#endif


/*
 * -------------------------------------------------------------------------
 * Production buddy lookup
 * -------------------------------------------------------------------------
 */

#if defined(CONFIG_MALLOC_BUDDY_FIBONACCI_LAYOUT)

/*
 * In the layout implementation, the sibling pointer is authoritative.
 *
 * No free-list search is necessary.
 */
struct fib_block *
fib_find_buddy(struct fib_block *block)
{
    struct fib_block *buddy = block->sibling;

    if (!buddy)
        return NULL;

    if (!buddy->free)
        return NULL;

    return buddy;
}

#endif


/*
 * -------------------------------------------------------------------------
 * Split
 * -------------------------------------------------------------------------
 */

struct fib_block *
fib_split(struct fib_block *block)
{
    unsigned int order = block->order;

    if (order < 2)
        return NULL;

    fib_list_remove(order, block);

    /*
     * F[n] -> F[n-1] + F[n-2]
     */
    unsigned int left_order = order - 1;
    unsigned int right_order = order - 2;

    struct fib_block *left = block;

    struct fib_block *right =
        (struct fib_block *)(
            (unsigned char *)block + fib_size[left_order]
        );

    left->order = left_order;
    left->free = false;

    right->order = right_order;
    right->next = NULL;
    right->prev = NULL;
    right->free = true;

#if defined(CONFIG_MALLOC_BUDDY_FIBONACCI_LAYOUT)

    left->parent = block->parent;
    right->parent = block->parent;

    left->sibling = right;
    right->sibling = left;

#endif

    fib_list_insert(right_order, right);

    return left;
}


/*
 * -------------------------------------------------------------------------
 * Initialization
 * -------------------------------------------------------------------------
 */

/*
 * Initialize the allocator over caller-provided storage.
 *
 * The arena must:
 *
 *     - be at least fib_arena_size() bytes;
 *     - be suitably aligned for struct fib_block.
 *
 * The allocator uses only fib_arena_size() bytes of the supplied arena.
 *
 * The caller retains ownership of the arena and must keep it alive for
 * the lifetime of all allocations made from this allocator.
 */
bool
fib_init(void *arena, size_t arena_size)
{
    if (!arena)
        return false;

    fib_init_sizes();

    size_t required = fib_size[FIB_CLASSES - 1];

    if (arena_size < required)
        return false;

    /*
     * Reset allocator state. This permits the same arena to be reused
     * between benchmark runs.
     */
    fib_heap = arena;
    fib_heap_size = arena_size;

    for (unsigned int i = 0; i < FIB_CLASSES; ++i)
        fib_free_list[i] = NULL;

    struct fib_block *root =
        (struct fib_block *)fib_heap;

    root->next = NULL;
    root->prev = NULL;
    root->order = FIB_CLASSES - 1;
    root->free = true;

#if defined(CONFIG_MALLOC_BUDDY_FIBONACCI_LAYOUT)

    root->parent = NULL;
    root->sibling = NULL;

#endif

    fib_list_insert(root->order, root);

    fib_initialized = true;

    return true;
}


/*
 * -------------------------------------------------------------------------
 * Allocation
 * -------------------------------------------------------------------------
 */

void *
buddy_malloc(size_t size)
{
    if (!fib_initialized || size == 0)
        return NULL;

    if (size > SIZE_MAX - FIB_HEADER_SIZE)
        return NULL;

    size += FIB_HEADER_SIZE;

    unsigned int wanted = fib_order_for_size(size);

    if (wanted >= FIB_CLASSES)
        return NULL;

    /*
     * Find the smallest available class at or above the requested class.
     */
    unsigned int order = wanted;

    while (order < FIB_CLASSES &&
           fib_free_list[order] == NULL)
        ++order;

    if (order >= FIB_CLASSES)
        return NULL;

    struct fib_block *block =
        fib_list_take(order);

    /*
     * Repeatedly split.
     *
     * The caller retains the larger/lower child.
     */
    while (order > wanted) {
        block->order = order;

        block = fib_split(block);

        if (!block)
            return NULL;

        --order;
    }

    block->free = false;

    return (unsigned char *)block + FIB_HEADER_SIZE;
}


/*
 * -------------------------------------------------------------------------
 * Deallocation
 * -------------------------------------------------------------------------
 */

void
buddy_free(void *ptr)
{
    if (!fib_initialized || !ptr)
        return;

    struct fib_block *block =
        (struct fib_block *)(
            (unsigned char *)ptr - FIB_HEADER_SIZE
        );

    unsigned int order = block->order;

    block->free = true;

    /*
     * Merge repeatedly while the Fibonacci buddy is free.
     */
    while (order < FIB_CLASSES - 1) {
        struct fib_block *buddy =
            fib_find_buddy(block);

        if (!buddy)
            break;

        unsigned int buddy_order = buddy->order;

        fib_list_remove(buddy_order, buddy);

#if defined(CONFIG_MALLOC_BUDDY_FIBONACCI_LAYOUT)

        /*
         * The merged block is represented by the lower-address child.
         */
        struct fib_block *parent;

        if ((uintptr_t)buddy < (uintptr_t)block)
            parent = buddy;
        else
            parent = block;

        ++order;

        parent->order = order;
        parent->free = true;

        /*
         * The children are now represented by the merged block.
         */
        parent->sibling = NULL;

        block = parent;

#else

        /*
         * Wave 1 has no explicit layout metadata.
         *
         * The lower-address block becomes the merged block.
         */
        if ((uintptr_t)buddy < (uintptr_t)block)
            block = buddy;

        ++order;
        block->order = order;
        block->free = true;

#endif
    }

    fib_list_insert(order, block);
}
