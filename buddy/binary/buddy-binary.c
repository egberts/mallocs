#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/*
 * Binary buddy allocator
 *
 * Block sizes:
 *
 *     2^MIN_ORDER ... 2^MAX_ORDER
 *
 * Every block has exactly one buddy:
 *
 *     buddy = address ^ block_size
 *
 * The allocator uses one free list per order.
 */

#ifndef CONFIG_MALLOC_BUDDY_MIN_ORDER
#define CONFIG_MALLOC_BUDDY_MIN_ORDER 5
#endif

#ifndef CONFIG_MALLOC_BUDDY_MAX_ORDER
#define CONFIG_MALLOC_BUDDY_MAX_ORDER 20
#endif

#define BUDDY_MIN_ORDER CONFIG_MALLOC_BUDDY_MIN_ORDER
#define BUDDY_MAX_ORDER CONFIG_MALLOC_BUDDY_MAX_ORDER

#define BUDDY_MIN_SIZE ((size_t)1 << BUDDY_MIN_ORDER)
#define BUDDY_MAX_SIZE ((size_t)1 << BUDDY_MAX_ORDER)
#define BUDDY_ORDERS   (BUDDY_MAX_ORDER - BUDDY_MIN_ORDER + 1)

/*
 * The heap must be naturally aligned to its maximum block size.
 *
 * A real implementation would normally obtain this memory from the
 * surrounding memory-management system rather than use a static array.
 */
static union {
    uint8_t bytes[BUDDY_MAX_SIZE];
    max_align_t alignment;
} buddy_heap;


/*
 * Free-list node.
 *
 * The node occupies the beginning of a free block.
 */
struct buddy_free {
    struct buddy_free *next;
    struct buddy_free *prev;
};


/*
 * Allocation header.
 *
 * This is stored immediately before the address returned to the caller.
 *
 * For a production allocator, the header representation would likely
 * be optimized substantially.
 */
struct buddy_header {
    uint8_t order;
    uint8_t allocated;
};


/*
 * One free list for each block order.
 */
static struct buddy_free *free_lists[BUDDY_ORDERS];

static bool buddy_initialized;


/*
 * Convert an order to a free-list index.
 */
static inline unsigned
buddy_index(unsigned order)
{
    return order - BUDDY_MIN_ORDER;
}


/*
 * Return the block size for an order.
 */
static inline size_t
buddy_size(unsigned order)
{
    return (size_t)1 << order;
}


/*
 * Convert a block address into the address of its buddy.
 *
 * Because the heap is aligned to BUDDY_MAX_SIZE, XOR with the
 * block size is sufficient.
 */
static inline uintptr_t
buddy_address(uintptr_t address, unsigned order)
{
    return address ^ buddy_size(order);
}


/*
 * Add a block to the free list for its order.
 */
static void
buddy_free_list_add(unsigned order, struct buddy_free *block)
{
    unsigned index = buddy_index(order);

    block->prev = NULL;
    block->next = free_lists[index];

    if (block->next != NULL)
        block->next->prev = block;

    free_lists[index] = block;
}


/*
 * Remove a block from its free list.
 */
static void
buddy_free_list_remove(unsigned order, struct buddy_free *block)
{
    unsigned index = buddy_index(order);

    if (block->prev != NULL)
        block->prev->next = block->next;
    else
        free_lists[index] = block->next;

    if (block->next != NULL)
        block->next->prev = block->prev;

    block->next = NULL;
    block->prev = NULL;
}


/*
 * Find and remove the first block from a free list.
 */
static struct buddy_free *
buddy_free_list_take(unsigned order)
{
    unsigned index = buddy_index(order);
    struct buddy_free *block = free_lists[index];

    if (block != NULL)
        buddy_free_list_remove(order, block);

    return block;
}


/*
 * Initialize the allocator.
 */
static void
buddy_init(void)
{
    struct buddy_free *block;

    if (buddy_initialized)
        return;

    for (unsigned i = 0; i < BUDDY_ORDERS; ++i)
        free_lists[i] = NULL;

    block = (struct buddy_free *)buddy_heap.bytes;

    buddy_free_list_add(BUDDY_MAX_ORDER, block);

    buddy_initialized = true;
}


/*
 * Determine the smallest block order capable of holding SIZE bytes.
 *
 * The allocation header is included in the calculation.
 */
static unsigned
buddy_order_for_size(size_t size)
{
    unsigned order = BUDDY_MIN_ORDER;
    size_t required = size + sizeof(struct buddy_header);

    if (required > BUDDY_MAX_SIZE)
        return BUDDY_MAX_ORDER + 1;

    while (buddy_size(order) < required) {
        ++order;

        if (order > BUDDY_MAX_ORDER)
            return order;
    }

    return order;
}


/*
 * Split a block until it reaches TARGET_ORDER.
 *
 * The first half is returned.
 * The second half is placed on the next lower-order free list.
 */
static struct buddy_free *
buddy_split(struct buddy_free *block,
            unsigned order,
            unsigned target_order)
{
    while (order > target_order) {
        unsigned child_order = order - 1;
        size_t child_size = buddy_size(child_order);
        uint8_t *address = (uint8_t *)block;
        struct buddy_free *buddy;

        --order;

        buddy = (struct buddy_free *)(address + child_size);

        buddy_free_list_add(order, buddy);
    }

    return block;
}


/*
 * Allocate SIZE bytes.
 */
void *
buddy_malloc(size_t size)
{
    unsigned target_order;
    unsigned order;
    struct buddy_free *block;
    struct buddy_header *header;

    if (size == 0)
        return NULL;

    buddy_init();

    target_order = buddy_order_for_size(size);

    if (target_order > BUDDY_MAX_ORDER)
        return NULL;

    /*
     * Find the smallest available block at or above target_order.
     */
    for (order = target_order;
         order <= BUDDY_MAX_ORDER;
         ++order) {
        block = buddy_free_list_take(order);

        if (block != NULL)
            break;
    }

    if (order > BUDDY_MAX_ORDER)
        return NULL;

    /*
     * Split the larger block until it reaches the required order.
     */
    block = buddy_split(block, order, target_order);

    header = (struct buddy_header *)block;

    header->order = target_order;
    header->allocated = 1;

    return (uint8_t *)header + sizeof(*header);
}


/*
 * Free SIZE's corresponding allocation.
 */
void
buddy_free(void *ptr)
{
    struct buddy_header *header;
    uint8_t *block;
    unsigned order;

    if (ptr == NULL)
        return;

    header = (struct buddy_header *)
        ((uint8_t *)ptr - sizeof(*header));

    if (!header->allocated)
        return;

    block = (uint8_t *)header;
    order = header->order;

    header->allocated = 0;

    /*
     * Coalesce with free buddies for as long as possible.
     */
    while (order < BUDDY_MAX_ORDER) {
        uintptr_t address = (uintptr_t)block;
        uintptr_t buddy_address_value;
        struct buddy_free *buddy;

        buddy_address_value = buddy_address(address, order);

        /*
         * The buddy must lie within our heap.
         */
        if (buddy_address_value <
            (uintptr_t)buddy_heap.bytes ||
            buddy_address_value >=
            (uintptr_t)buddy_heap.bytes + BUDDY_MAX_SIZE)
            break;

        buddy = (struct buddy_free *)buddy_address_value;

        /*
         * This prototype uses a small allocation marker in the
         * block header to determine whether the buddy is allocated.
         *
         * A free block has its free-list links occupying the same
         * storage, so determine membership by searching its list.
         */
        {
            struct buddy_free *candidate;
            bool found = false;

            for (candidate = free_lists[buddy_index(order)];
                 candidate != NULL;
                 candidate = candidate->next) {
                if (candidate == buddy) {
                    found = true;
                    break;
                }
            }

            if (!found)
                break;
        }

        /*
         * Remove the buddy and merge the two blocks.
         */
        buddy_free_list_remove(order, buddy);

        if (buddy_address_value < (uintptr_t)block)
            block = (uint8_t *)buddy;

        ++order;
    }

    buddy_free_list_add(order, (struct buddy_free *)block);
}
