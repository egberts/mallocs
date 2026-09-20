#ifndef _BUDDY_BINARY_H
#define _BUDDY_BINARY_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

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

union {
    uint8_t bytes[BUDDY_MAX_SIZE];
    max_align_t alignment;
} buddy_heap;

struct buddy_free {
    struct buddy_free *next;
    struct buddy_free *prev;
};

struct buddy_header {
    uint8_t order;
    uint8_t allocated;
};


extern struct buddy_free *free_lists[BUDDY_ORDERS];
extern bool buddy_initialized;


extern void buddy_free_list_add(unsigned order, struct buddy_free *block);
extern void buddy_free_list_remove(unsigned order, struct buddy_free *block);
extern struct buddy_free * buddy_free_list_take(unsigned order);
extern void buddy_init(void);
extern unsigned buddy_order_for_size(size_t size);
extern struct buddy_free *buddy_split(struct buddy_free *block, unsigned order, unsigned target_order);
extern void *buddy_malloc(size_t size);
extern void buddy_free(void *ptr);

#endif  // _BUDDY_BINARY_H
