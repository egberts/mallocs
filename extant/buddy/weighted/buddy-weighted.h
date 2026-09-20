#ifndef  _BUDDY_WEIGHTED_H
#define  _BUDDY_WEIGHTED_H
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

/*
 * ----------------------------------------------------------------------
 * Block metadata
 * ----------------------------------------------------------------------
 */

struct wb_block {
    struct wb_block *next;
    struct wb_block *prev;

    struct wb_block *parent;
    struct wb_block *left;
    struct wb_block *right;

    size_t size;

    unsigned free : 1;
    unsigned leaf : 1;
};


/*
 * ----------------------------------------------------------------------
 * Free classes
 * ----------------------------------------------------------------------
 */

struct wb_class {
    struct wb_block *free;
    size_t size;
};


/*
 * ----------------------------------------------------------------------
 * Allocation policy
 * ----------------------------------------------------------------------
 */

enum weighted_buddy_policy {
    WEIGHTED_BUDDY_FIRST_FIT,
};


struct weighted_buddy {
    void *base;
    size_t size;

    struct wb_block *root;

    struct wb_class *classes;
    size_t nr_classes;

    enum weighted_buddy_policy policy;
};


extern size_t weighted_buddy_class_size(size_t class);
extern size_t weighted_buddy_class_for(size_t size, size_t nr_classes);
extern void wb_list_insert(struct wb_class *class, struct wb_block *block);
extern void wb_list_remove(struct wb_class *class, struct wb_block *block);
extern struct wb_block *wb_policy_first_fit(struct weighted_buddy *wb, size_t class);
extern struct wb_block *weighted_buddy_choose_block(struct weighted_buddy *wb, size_t class);
extern int wb_split(struct weighted_buddy *wb, struct wb_block *block);
extern struct wb_block * wb_choose_child(struct wb_block *block, size_t requested_class);
extern struct wb_block *wb_choose_child(struct wb_block *block, size_t requested_class);
extern void * wb_commit(struct weighted_buddy *wb, struct wb_block *block);
extern bool wb_children_free(struct wb_block *block);
extern void wb_coalesce(struct weighted_buddy *wb, struct wb_block *block);

extern void *buddy_alloc(size_t size);
extern void buddy_free(void *ptr);

#endif  // _BUDDY_WEIGHTED_H


