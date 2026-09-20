

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "config.h"

#ifndef CONFIG_MALLOC_BUDDY_FIBONACCI_BASE
#define CONFIG_MALLOC_BUDDY_FIBONACCI_BASE 16
#endif

#ifndef CONFIG_MALLOC_BUDDY_FIBONACCI_CLASSES
#define CONFIG_MALLOC_BUDDY_FIBONACCI_CLASSES 32
#endif

#define FIB_BASE    CONFIG_MALLOC_BUDDY_FIBONACCI_BASE
#define FIB_CLASSES CONFIG_MALLOC_BUDDY_FIBONACCI_CLASSES


size_t fib_size[FIB_CLASSES];


void fib_init_sizes(void);
size_t fib_arena_size(void);


struct fib_block {
    struct fib_block *next;
    struct fib_block *prev;

    unsigned int order;
    bool free;

#if defined(CONFIG_MALLOC_BUDDY_FIBONACCI_LAYOUT)
    struct fib_block *parent;
    struct fib_block *sibling;

#endif
};

#define FIB_HEADER_SIZE \
    (((sizeof(struct fib_block) + FIB_BASE - 1) / FIB_BASE) * FIB_BASE)


unsigned char *fib_heap;
size_t fib_heap_size;
bool fib_initialized;
struct fib_block *fib_free_list[FIB_CLASSES];

void fib_list_insert(unsigned int order, struct fib_block *block);
void fib_list_remove(unsigned int order, struct fib_block *block);
struct fib_block *fib_list_take(unsigned int order);
 unsigned int fib_order_for_size(size_t size);

#if defined(CONFIG_MALLOC_BUDDY_FIBONACCI_SEARCH)
struct fib_block *fib_find_free(unsigned int order, uintptr_t address);
struct fib_block *fib_find_buddy(struct fib_block *block);
#endif

#if defined(CONFIG_MALLOC_BUDDY_FIBONACCI_LAYOUT)
struct fib_block *fib_find_buddy(struct fib_block *block);
#endif


struct fib_block *fib_split(struct fib_block *block);
bool fib_init(void *arena, size_t arena_size);
void *buddy_malloc(size_t size);
void buddy_free(void *ptr);
