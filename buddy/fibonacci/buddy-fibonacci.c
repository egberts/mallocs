#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/*
 * Fibonacci buddy allocator
 *
 * Block sizes:
 *
 *     F[0] = 1
 *     F[1] = 2
 *     F[n] = F[n - 1] + F[n - 2]
 *
 * Actual block size is:
 *
 *     F[n] * BUDDY_MIN_SIZE
 *
 * Thus:
 *
 *     1, 2, 3, 5, 8, 13, 21, 34, ...
 *
 * Every split produces:
 *
 *     F[n]
 *       |
 *       +-- F[n - 1]  (lower address)
 *       |
 *       +-- F[n - 2]  (higher address)
 *
 * Unlike binary buddy, Fibonacci buddy addresses cannot be
 * determined with a simple XOR operation.
 */

#ifndef CONFIG_MALLOC_BUDDY_MIN_ORDER
#define CONFIG_MALLOC_BUDDY_MIN_ORDER 5
#endif

#ifndef CONFIG_MALLOC_BUDDY_FIBONACCI_MAX_ORDER
#define CONFIG_MALLOC_BUDDY_FIBONACCI_MAX_ORDER 20
#endif

#define FIB_MIN_ORDER 0
#define FIB_MAX_ORDER CONFIG_MALLOC_BUDDY_FIBONACCI_MAX_ORDER

#define FIB_MIN_SIZE ((size_t)1 << CONFIG_MALLOC_BUDDY_MIN_ORDER)
#define FIB_ORDERS   (FIB_MAX_ORDER + 1)


/*
 * Fibonacci block sizes, expressed as multiples of the minimum
 * allocation unit.
 *
 * F[0] = 1
 * F[1] = 2
 * F[n] = F[n - 1] + F[n - 2]
 */
static size_t fib_size[FIB_ORDERS];


/*
 * The heap consists of one maximum-sized Fibonacci block.
 */
static union {
    uint8_t bytes[FIB_MIN_SIZE * 17711];
    max_align_t alignment;
} fib_heap;


/*
 * Free-list node.
 *
 * The node occupies the beginning of a free block.
 */
struct fib_free {
    struct fib_free *next;
    struct fib_free *prev;
};


/*
 * Allocation header.
 *
 * order identifies the Fibonacci size class.
 *
 * allocated is kept for diagnostics and double-free detection.
 */
struct fib_header {
    uint8_t order;
    uint8_t allocated;
};


/*
 * One free list for each Fibonacci size.
 */
static struct fib_free *free_lists[FIB_ORDERS];

static bool fib_initialized;


/*
 * Calculate the Fibonacci size table.
 */
static void
fib_init_sizes(void)
{
    fib_size[0] = 1;

    if (FIB_ORDERS > 1)
        fib_size[1] = 2;

    for (unsigned i = 2; i < FIB_ORDERS; ++i)
        fib_size[i] = fib_size[i - 1] + fib_size[i - 2];
}


/*
 * Return the byte size of a Fibonacci block.
 */
static inline size_t
fib_block_size(unsigned order)
{
    return fib_size[order] * FIB_MIN_SIZE;
}


/*
 * Add a block to a Fibonacci free list.
 */
static void
fib_free_list_add(unsigned order, struct fib_free *block)
{
    block->prev = NULL;
    block->next = free_lists[order];

    if (block->next != NULL)
        block->next->prev = block;

    free_lists[order] = block;
}


/*
 * Remove a block from a Fibonacci free list.
 */
static void
fib_free_list_remove(unsigned order, struct fib_free *block)
{
    if (block->prev != NULL)
        block->prev->next = block->next;
    else
        free_lists[order] = block->next;

    if (block->next != NULL)
        block->next->prev = block->prev;

    block->next = NULL;
    block->prev = NULL;
}


/*
 * Remove and return the first block from a free list.
 */
static struct fib_free *
fib_free_list_take(unsigned order)
{
    struct fib_free *block = free_lists[order];

    if (block != NULL)
        fib_free_list_remove(order, block);

    return block;
}


/*
 * Initialize the allocator.
 */
static void
fib_init(void)
{
    struct fib_free *block;

    if (fib_initialized)
        return;

    fib_init_sizes();

    for (unsigned i = 0; i < FIB_ORDERS; ++i)
        free_lists[i] = NULL;

    block = (struct fib_free *)fib_heap.bytes;

    fib_free_list_add(FIB_MAX_ORDER, block);

    fib_initialized = true;
}


/*
 * Find the smallest Fibonacci block capable of holding SIZE bytes.
 */
static unsigned
fib_order_for_size(size_t size)
{
    size_t required = size + sizeof(struct fib_header);

    for (unsigned order = FIB_MIN_ORDER;
         order <= FIB_MAX_ORDER;
         ++order) {
        if (fib_block_size(order) >= required)
            return order;
    }

    return FIB_MAX_ORDER + 1;
}


/*
 * Split an order-N block.
 *
 * F[n] = F[n-1] + F[n-2]
 *
 * The larger child is placed at the lower address.
 *
 * The child being returned remains at the lower address.
 * The smaller child is inserted into its free list.
 */
static struct fib_free *
fib_split(struct fib_free *block,
          unsigned order,
          unsigned target_order)
{
    uint8_t *address = (uint8_t *)block;

    while (order > target_order) {
        unsigned left_order = order - 1;
        unsigned right_order = order - 2;

        /*
         * For the first two classes there is no meaningful
         * F[n-2] split. The minimum useful split is F[2] =
         * F[1] + F[0] = 2 + 1.
         */
        if (right_order < FIB_MIN_ORDER)
            break;

        /*
         * The lower-address child has size F[n-1].
         */
        size_t left_size = fib_block_size(left_order);

        struct fib_free *right =
            (struct fib_free *)(address + left_size);

        --order;

        fib_free_list_add(right_order, right);
    }

    return block;
}


/*
 * Find a free block at ADDRESS in the specified order.
 *
 * This is deliberately linear for the prototype.
 *
 * A production implementation would maintain enough metadata to
 * determine buddy state without traversing the free list.
 */
static struct fib_free *
fib_find_free(unsigned order, uintptr_t address)
{
    struct fib_free *block;

    for (block = free_lists[order];
         block != NULL;
         block = block->next) {
        if ((uintptr_t)block == address)
            return block;
    }

    return NULL;
}


/*
 * Determine the Fibonacci buddy of BLOCK.
 *
 * The original Fibonacci buddy scheme puts the larger child at the
 * lower address.
 *
 * A block of order N can have a buddy of order N+1 or N-1:
 *
 *     [ F[N+1] ]
 *     [ F[N] ][ F[N-1] ]
 *
 * If BLOCK is the larger, lower-address child:
 *
 *     buddy = address + size(BLOCK)
 *
 * If BLOCK is the smaller, higher-address child:
 *
 *     buddy = address - size(parent)
 *
 * The heap layout is used to determine which case applies.
 */
static struct fib_free *
fib_find_buddy(uint8_t *block, unsigned order,
               unsigned *buddy_order)
{
    uintptr_t address = (uintptr_t)block;
    uintptr_t heap_start = (uintptr_t)fib_heap.bytes;
    uintptr_t relative = address - heap_start;

    /*
     * First possibility:
     *
     * BLOCK is the larger F[n] child of an F[n+1] parent.
     *
     * Its buddy is F[n-1] immediately after it.
     */
    if (order >= 2 && order + 1 <= FIB_MAX_ORDER) {
        unsigned candidate_order = order - 1;
        uintptr_t buddy_address =
            address + fib_block_size(order);

        struct fib_free *buddy =
            fib_find_free(candidate_order, buddy_address);

        if (buddy != NULL) {
            /*
             * Verify that this is actually a valid
             * Fibonacci boundary.
             */
            uintptr_t parent_offset =
                relative;

            (void)parent_offset;

            *buddy_order = candidate_order;
            return buddy;
        }
    }

    /*
     * Second possibility:
     *
     * BLOCK is the smaller F[n] child of an F[n+1] parent.
     *
     * Its buddy is the preceding F[n+1] block.
     *
     * The preceding block has order n+1.
     */
    if (order + 1 <= FIB_MAX_ORDER) {
        unsigned candidate_order = order + 1;

        if (address >= heap_start +
            fib_block_size(candidate_order)) {
            uintptr_t buddy_address =
                address - fib_block_size(candidate_order);

            struct fib_free *buddy =
                fib_find_free(candidate_order,
                          buddy_address);

            if (buddy != NULL) {
                *buddy_order = candidate_order;
                return buddy;
            }
        }
    }

    return NULL;
}


/*
 * Allocate SIZE bytes.
 */
void *
fib_malloc(size_t size)
{
    unsigned target_order;
    unsigned order;
    struct fib_free *block;
    struct fib_header *header;

    if (size == 0)
        return NULL;

    fib_init();

    target_order = fib_order_for_size(size);

    if (target_order > FIB_MAX_ORDER)
        return NULL;

    /*
     * Find the smallest available Fibonacci block at or above
     * the requested size.
     */
    for (order = target_order;
         order <= FIB_MAX_ORDER;
         ++order) {
        block = fib_free_list_take(order);

        if (block != NULL)
            break;
    }

    if (order > FIB_MAX_ORDER)
        return NULL;

    /*
     * Repeatedly split until the requested Fibonacci class is
     * reached.
     */
    block = fib_split(block, order, target_order);

    header = (struct fib_header *)block;

    header->order = target_order;
    header->allocated = 1;

    return (uint8_t *)header + sizeof(*header);
}


/*
 * Free an allocation.
 */
void
fib_free(void *ptr)
{
    struct fib_header *header;
    uint8_t *block;
    unsigned order;

    if (ptr == NULL)
        return;

    header = (struct fib_header *)
        ((uint8_t *)ptr - sizeof(*header));

    if (!header->allocated)
        return;

    block = (uint8_t *)header;
    order = header->order;

    header->allocated = 0;

    /*
     * Repeatedly merge with a free Fibonacci buddy.
     */
    while (order < FIB_MAX_ORDER) {
        unsigned buddy_order;
        struct fib_free *buddy;

        buddy = fib_find_buddy(block, order, &buddy_order);

        if (buddy == NULL)
            break;

        fib_free_list_remove(buddy_order, buddy);

        /*
         * The larger Fibonacci child is always the lower-address
         * child. The resulting parent therefore starts at the
         * lower of the two addresses.
         */
        if ((uintptr_t)buddy < (uintptr_t)block)
            block = (uint8_t *)buddy;

        /*
         * The parent is one Fibonacci class larger than the
         * larger child.
         */
        if (buddy_order > order)
            order = buddy_order;

        ++order;
    }

    fib_free_list_add(order, (struct fib_free *)block);
}
