/*
 * buddy-weighted.c
 *
 * Weighted buddy allocator subsystem.
 *
 * Size classes:
 *
 *     1, 2, 3, 4, 6, 8, 12, 16, 24, 32, ...
 *
 * The allocator uses the classic 1:2 weighted-buddy decomposition.
 *
 * This is intentionally a subsystem rather than a replacement for
 * malloc()/free().  The caller supplies an arena and may place whatever
 * policy it wants above this layer.
 *
 * This subsystem deliberately does not provide malloc()/free().
 * Higher-level allocation policy belongs above this layer.
 */

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "config.h"
#include "buddy-weighted.h"




static struct weighted_buddy weighted_buddy;

/*
 * ----------------------------------------------------------------------
 * Size selector
 * ----------------------------------------------------------------------
 *
 * Weighted-buddy classes alternate:
 *
 *     2^k
 *     3 * 2^k
 *
 * giving:
 *
 *     1
 *     2
 *     3
 *     4
 *     6
 *     8
 *     12
 *     16
 *     24
 *     32
 *     ...
 */

size_t
weighted_buddy_class_size(size_t class)
{
    size_t order;
    size_t size;

    order = class / 2;
    size = (size_t)1 << order;

    if (class & 1)
        size += size / 2;

    return size;
}


/*
 * Return the smallest class capable of containing 'size'.
 *
 * SIZE_MAX indicates that no configured class can satisfy the
 * requested size.
 */

size_t
weighted_buddy_class_for(size_t size, size_t nr_classes)
{
    size_t class;

    if (!size)
        return SIZE_MAX;

    for (class = 0; class < nr_classes; ++class) {
        if (weighted_buddy_class_size(class) >= size)
            return class;
    }

    return SIZE_MAX;
}


/*
 * ----------------------------------------------------------------------
 * Free-list operations
 * ----------------------------------------------------------------------
 */

void
wb_list_insert(struct wb_class *class, struct wb_block *block)
{
    block->prev = NULL;
    block->next = class->free;

    if (class->free)
        class->free->prev = block;

    class->free = block;
    block->free = 1;
}


void
wb_list_remove(struct wb_class *class, struct wb_block *block)
{
    if (block->prev)
        block->prev->next = block->next;
    else
        class->free = block->next;

    if (block->next)
        block->next->prev = block->prev;

    block->next = NULL;
    block->prev = NULL;
    block->free = 0;
}


/*
 * ----------------------------------------------------------------------
 * Allocation policy
 * ----------------------------------------------------------------------
 *
 * The policy receives the already-selected size class.
 *
 * It does not calculate size classes and does not perform splitting.
 */

struct wb_block *
wb_policy_first_fit(struct weighted_buddy *wb, size_t class)
{
    size_t i;

    for (i = class; i < wb->nr_classes; ++i) {
        if (wb->classes[i].free)
            return wb->classes[i].free;
    }

    return NULL;
}


struct wb_block *
weighted_buddy_choose_block(struct weighted_buddy *wb,
                size_t class)
{
    switch (wb->policy) {
    case WEIGHTED_BUDDY_FIRST_FIT:
        return wb_policy_first_fit(wb, class);

    default:
        return NULL;
    }
}


/*
 * ----------------------------------------------------------------------
 * Split
 * ----------------------------------------------------------------------
 *
 * Split one specific block.
 *
 * The block itself becomes an internal tree node.
 * The caller chooses which child to continue allocating from.
 *
 * The children are inserted into their respective free lists.
 */

int
wb_split(struct weighted_buddy *wb,
     struct wb_block *block)
{
    size_t class;
    size_t left_size;
    size_t right_size;
    struct wb_block *left;
    struct wb_block *right;

    if (!block->leaf)
        return -1;

    class = weighted_buddy_class_for(
        block->size,
        wb->nr_classes
    );

    if (class == SIZE_MAX || class == 0)
        return -1;

    /*
     * Remove the parent from its free list.
     */
    wb_list_remove(
        &wb->classes[class],
        block
    );

    if (class & 1) {
        /*
         * 3 * 2^k
         *
         * -> 2^k + 2^(k+1)
         */
        left_size = block->size / 3;
        right_size = block->size - left_size;
    } else {
        /*
         * 2^k
         *
         * -> 2^(k-1) + 2^(k-1)
         */
        left_size = block->size / 2;
        right_size = left_size;
    }

    /*
     * Metadata allocation is deliberately kept separate from the
     * allocation policy in this version.
     *
     * This demonstration stores the child metadata immediately
     * following the parent metadata.
     */
    left = (struct wb_block *)(
        (uint8_t *)block + sizeof(*block)
    );

    right = left + 1;

    left->next = NULL;
    left->prev = NULL;
    left->parent = block;
    left->left = NULL;
    left->right = NULL;
    left->size = left_size;
    left->free = 1;
    left->leaf = 1;

    right->next = NULL;
    right->prev = NULL;
    right->parent = block;
    right->left = NULL;
    right->right = NULL;
    right->size = right_size;
    right->free = 1;
    right->leaf = 1;

    block->left = left;
    block->right = right;
    block->leaf = 0;
    block->free = 0;

    wb_list_insert(
        &wb->classes[
            weighted_buddy_class_for(
                left_size,
                wb->nr_classes
            )
        ],
        left
    );

    wb_list_insert(
        &wb->classes[
            weighted_buddy_class_for(
                right_size,
                wb->nr_classes
            )
        ],
        right
    );

    return 0;
}


/*
 * ----------------------------------------------------------------------
 * Select child after a split
 * ----------------------------------------------------------------------
 *
 * Both children are valid candidates.  Continue with the child whose
 * class can satisfy the original request.
 *
 * If both children satisfy it, select the smaller child.
 */

struct wb_block *
wb_choose_child(struct wb_block *block, size_t requested_class)
{
    struct wb_block *left;
    struct wb_block *right;

    left = block->left;
    right = block->right;

    if (left->size >= weighted_buddy_class_size(requested_class) &&
        right->size >= weighted_buddy_class_size(requested_class)) {

        if (left->size <= right->size)
            return left;

        return right;
    }

    if (left->size >= weighted_buddy_class_size(requested_class))
        return left;

    if (right->size >= weighted_buddy_class_size(requested_class))
        return right;

    return NULL;
}


/*
 * ----------------------------------------------------------------------
 * Commit allocation
 * ----------------------------------------------------------------------
 */

void *
wb_commit(struct weighted_buddy *wb,
      struct wb_block *block)
{
    size_t class;

    class = weighted_buddy_class_for(
        block->size,
        wb->nr_classes
    );

    if (class == SIZE_MAX)
        return NULL;

    wb_list_remove(
        &wb->classes[class],
        block
    );

    block->free = 0;

    return (uint8_t *)block + sizeof(*block);
}


/*
 * ----------------------------------------------------------------------
 * Allocation
 * ----------------------------------------------------------------------
 */
void *
buddy_alloc(size_t size)
{
    struct weighted_buddy *wb = &weighted_buddy;
    size_t requested_class;
    size_t block_class;
    struct wb_block *block;

    if (size == 0)
        return NULL;

    requested_class = weighted_buddy_class_for(
        size,
        wb->nr_classes
    );

    if (requested_class == SIZE_MAX)
        return NULL;

    block = weighted_buddy_choose_block(
        wb,
        requested_class
    );

    if (!block)
        return NULL;

    for (;;) {
        block_class = weighted_buddy_class_for(
            block->size,
            wb->nr_classes
        );

        if (block_class == requested_class)
            break;

        if (block_class < requested_class)
            return NULL;

        if (wb_split(wb, block))
            return NULL;

        block = wb_choose_child(
            block,
            requested_class
        );

        if (!block)
            return NULL;
    }

    return wb_commit(wb, block);
}


/*
 * ----------------------------------------------------------------------
 * Coalescing
 * ----------------------------------------------------------------------
 */

bool
wb_children_free(struct wb_block *block)
{
    return block->left &&
           block->right &&
           block->left->leaf &&
           block->right->leaf &&
           block->left->free &&
           block->right->free;
}


void
wb_coalesce(struct weighted_buddy *wb,
        struct wb_block *block)
{
    struct wb_block *parent;
    size_t class;

    while (block->parent) {
        parent = block->parent;

        if (!wb_children_free(parent))
            break;

        class = weighted_buddy_class_for(
            parent->left->size,
            wb->nr_classes
        );

        wb_list_remove(
            &wb->classes[class],
            parent->left
        );

        class = weighted_buddy_class_for(
            parent->right->size,
            wb->nr_classes
        );

        wb_list_remove(
            &wb->classes[class],
            parent->right
        );

        parent->left = NULL;
        parent->right = NULL;
        parent->leaf = 1;
        parent->free = 1;

        class = weighted_buddy_class_for(
            parent->size,
            wb->nr_classes
        );

        wb_list_insert(
            &wb->classes[class],
            parent
        );

        block = parent;
    }
}


/*
 * ----------------------------------------------------------------------
 * Free
 * ----------------------------------------------------------------------
 */

void
buddy_free(void *ptr)
{
    size_t class;
    struct weighted_buddy *wb = &weighted_buddy;
    struct wb_block *block;

    if (!ptr)
        return;

    block = (struct wb_block *)(
        (uint8_t *)ptr - sizeof(*block)
    );

    /* ... mark free and coalesce ... */

    if (!wb || !ptr)
        return;

    block = (struct wb_block *)(
        (uint8_t *)ptr - sizeof(*block)
    );

    if (block->free)
        return;

    class = weighted_buddy_class_for(
        block->size,
        wb->nr_classes
    );

    if (class == SIZE_MAX)
        return;

    wb_list_insert(
        &wb->classes[class],
        block
    );

    wb_coalesce(wb, block);
}

