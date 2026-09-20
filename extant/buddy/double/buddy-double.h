#ifndef _BUDDY_DOUBLE_H
#define _BUDDY_DOUBLE_H
/*
 * double-buddy.c
 *
 * Double-buddy memory allocator.
 *
 * Two staggered buddy series:
 *
 *     A: BASE_A * 2^n
 *     B: BASE_B * 2^n
 *
 * Each series is internally a conventional binary buddy system.
 *
 * This implementation is intended as an allocator building block,
 * not as a production replacement for glibc malloc().
 *
 * Linux/POSIX.
 */

#define _GNU_SOURCE

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdalign.h>
#include <sys/mman.h>
#include <unistd.h>
#include "config.h"


/*
 * The two size series.
 *
 * 16, 32, 64, 128, ...
 * 24, 48, 96, 192, ...
 *
 * The ratio between adjacent classes in the combined sequence is
 * therefore considerably tighter than a conventional binary buddy.
 */
#define BASE_A          16
#define BASE_B          24

#define MIN_ORDER       0
#define MAX_ORDER       20

#define MAX_ORDERS      (MAX_ORDER + 1)

/*
 * A reasonably large arena.
 *
 * Each arena is independently managed.  The arena itself is a
 * power-of-two-sized region so the buddy arithmetic remains simple.
 */
#define ARENA_SIZE      (1UL << 22)       /* 4 MiB */
#define ARENA_MAGIC     UINT64_C(0x44424D454D414C4C)

/*
 * Free blocks are intrusive.
 *
 * The list pointers occupy the beginning of the free block itself.
 */
struct free_block {
    struct free_block *next;
    struct free_block *prev;
};

enum buddy_series {
    SERIES_A = 0,
    SERIES_B = 1
};

struct buddy_list {
    struct free_block *head;
};

struct double_buddy {
    void *base;
    size_t size;

    /*
     * One free-list hierarchy for each buddy series.
     */
    struct buddy_list free_a[MAX_ORDERS];
    struct buddy_list free_b[MAX_ORDERS];

    uint64_t magic;
};

/*
 * Allocation header.
 *
 * The header is immediately before the user pointer.
 *
 * The actual buddy block begins at 'block', while the user-visible
 * allocation begins after this header.
 */
struct allocation {
    uint64_t magic;

    struct double_buddy *arena;

    uintptr_t block;
    size_t block_size;

    enum buddy_series series;
    unsigned order;

    size_t mapping_size;
};

#define ALLOC_MAGIC UINT64_C(0x4442414C4C4F4341)

/*
 * ----------------------------------------------------------------------
 * Utility functions
 * ----------------------------------------------------------------------
 */

size_t series_size(enum buddy_series series, unsigned order);
size_t min_size(void);
unsigned ceil_order(enum buddy_series series, size_t size);
bool valid_order(unsigned order);
size_t next_series_size(enum buddy_series series, size_t size);
bool choose_class(size_t size, enum buddy_series *series, unsigned *order);
struct buddy_list * get_list(struct double_buddy *db, enum buddy_series series, unsigned order);
void list_insert(struct buddy_list *list, struct free_block *block);
void list_remove(struct buddy_list *list, struct free_block *block);
struct free_block * list_pop(struct buddy_list *list);
struct free_block * list_find(struct buddy_list *list, uintptr_t address);
uintptr_t buddy_address(struct double_buddy *db, enum buddy_series series, unsigned order, uintptr_t address);
struct double_buddy * arena_create(void);
void * buddy_alloc(struct double_buddy *db, enum buddy_series series, unsigned wanted_order);
void buddy_free_block(struct double_buddy *db, enum buddy_series series, unsigned order, uintptr_t address);
void * buddy_malloc(size_t size);
void buddy_free(void *ptr);
void * buddy_calloc(size_t nmemb, size_t size);
void * buddy_realloc(void *ptr, size_t size);

#endif  // _BUDDY_DOUBLE_H
