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
#include "buddy-double.h"

/*
 * ----------------------------------------------------------------------
 * Utility functions
 * ----------------------------------------------------------------------
 */

size_t
series_size(enum buddy_series series, unsigned order)
{
    size_t base;

    base = (series == SERIES_A) ? BASE_A : BASE_B;

    return base << order;
}

size_t
min_size(void)
{
    return sizeof(struct allocation) + BASE_A;
}

unsigned
ceil_order(enum buddy_series series, size_t size)
{
    unsigned order = 0;

    while (series_size(series, order) < size)
        order++;

    return order;
}

bool
valid_order(unsigned order)
{
    return order <= MAX_ORDER;
}

/*
 * Return the next supported size in a particular series.
 */
size_t
next_series_size(enum buddy_series series, size_t size)
{
    unsigned order;

    order = ceil_order(series, size);

    if (!valid_order(order))
        return SIZE_MAX;

    return series_size(series, order);
}

/*
 * Pick whichever double-buddy class wastes the least space.
 *
 * Ties go to series A.
 */
bool
choose_class(size_t size,
             enum buddy_series *series,
             unsigned *order)
{
    size_t a;
    size_t b;
    unsigned ao;
    unsigned bo;

    ao = ceil_order(SERIES_A, size);
    bo = ceil_order(SERIES_B, size);

    if (!valid_order(ao) && !valid_order(bo))
        return false;

    a = valid_order(ao) ? series_size(SERIES_A, ao) : SIZE_MAX;
    b = valid_order(bo) ? series_size(SERIES_B, bo) : SIZE_MAX;

    if (a <= b) {
        *series = SERIES_A;
        *order = ao;
    } else {
        *series = SERIES_B;
        *order = bo;
    }

    return true;
}

/*
 * ----------------------------------------------------------------------
 * Free-list manipulation
 * ----------------------------------------------------------------------
 */

struct buddy_list *
get_list(struct double_buddy *db,
         enum buddy_series series,
         unsigned order)
{
    return series == SERIES_A
        ? &db->free_a[order]
        : &db->free_b[order];
}

void
list_insert(struct buddy_list *list, struct free_block *block)
{
    block->prev = NULL;
    block->next = list->head;

    if (list->head)
        list->head->prev = block;

    list->head = block;
}

void
list_remove(struct buddy_list *list, struct free_block *block)
{
    if (block->prev)
        block->prev->next = block->next;
    else
        list->head = block->next;

    if (block->next)
        block->next->prev = block->prev;

    block->next = NULL;
    block->prev = NULL;
}

struct free_block *
list_pop(struct buddy_list *list)
{
    struct free_block *block;

    block = list->head;

    if (block)
        list_remove(list, block);

    return block;
}

/*
 * Search for an exact address in a free list.
 *
 * This is intentionally linear.  The allocator can later replace this
 * with a bitmap, address-indexed tree, radix structure, etc.
 */
struct free_block *
list_find(struct buddy_list *list, uintptr_t address)
{
    struct free_block *block;

    for (block = list->head; block; block = block->next) {
        if ((uintptr_t)block == address)
            return block;
    }

    return NULL;
}

/*
 * ----------------------------------------------------------------------
 * Buddy arithmetic
 * ----------------------------------------------------------------------
 *
 * The buddy relationship is relative to the beginning of the arena
 * belonging to the series.
 *
 * For a block at offset 'x' with size 's':
 *
 *     buddy_offset = x ^ s
 *
 * Because the two series have different base sizes, each series has
 * its own alignment lattice.
 *
 * The arena is therefore initialized with one root block for each
 * series at a suitable aligned offset.
 * ----------------------------------------------------------------------
 */

uintptr_t
buddy_address(struct double_buddy *db,
              enum buddy_series series,
              unsigned order,
              uintptr_t address)
{
    size_t size;
    uintptr_t offset;

    size = series_size(series, order);

    offset = address - (uintptr_t)db->base;

    offset ^= size;

    return (uintptr_t)db->base + offset;
}

/*
 * ----------------------------------------------------------------------
 * Arena initialization
 * ----------------------------------------------------------------------
 */

struct double_buddy *
arena_create(void)
{
    struct double_buddy *db;
    long page_size;

    page_size = sysconf(_SC_PAGESIZE);

    if (page_size <= 0)
        return NULL;

    db = mmap(NULL,
              ARENA_SIZE,
              PROT_READ | PROT_WRITE,
              MAP_PRIVATE | MAP_ANONYMOUS,
              -1,
              0);

    if (db == MAP_FAILED)
        return NULL;

    db->base = db;
    db->size = ARENA_SIZE;
    db->magic = ARENA_MAGIC;

    /*
     * Start the two buddy systems with independent roots.
     *
     * The root sizes are selected so that both systems can subdivide
     * the arena without overlapping.
     *
     * For this simple implementation, each series gets half the arena.
     */
    {
        uintptr_t base = (uintptr_t)db;
        size_t half = ARENA_SIZE / 2;

        /*
         * Series A root.
         */
        {
            unsigned order = 0;

            while (series_size(SERIES_A, order + 1) <= half)
                order++;

            {
                uintptr_t addr = base;

                /*
                 * The root is inserted as one free block.
                 */
                list_insert(
                    get_list(db, SERIES_A, order),
                    (struct free_block *)addr
                );
            }
        }

        /*
         * Series B root.
         */
        {
            unsigned order = 0;

            while (series_size(SERIES_B, order + 1) <= half)
                order++;

            {
                uintptr_t addr = base + half;

                list_insert(
                    get_list(db, SERIES_B, order),
                    (struct free_block *)addr
                );
            }
        }
    }

    return db;
}

/*
 * ----------------------------------------------------------------------
 * Allocation from one buddy system.
 * ----------------------------------------------------------------------
 */

void *
buddy_alloc(struct double_buddy *db,
            enum buddy_series series,
            unsigned wanted_order)
{
    unsigned order;
    struct free_block *block;
    uintptr_t address;

    /*
     * Find the first larger free class.
     */
    for (order = wanted_order; order <= MAX_ORDER; order++) {
        struct buddy_list *list;

        list = get_list(db, series, order);

        block = list_pop(list);

        if (block)
            goto found;
    }

    return NULL;

found:

    address = (uintptr_t)block;

    /*
     * Split downward.
     */
    while (order > wanted_order) {
        uintptr_t buddy;
        size_t half;

        order--;

        half = series_size(series, order);

        buddy = address + half;

        list_insert(
            get_list(db, series, order),
            (struct free_block *)buddy
        );
    }

    return (void *)address;
}

/*
 * ----------------------------------------------------------------------
 * Free / coalesce.
 * ----------------------------------------------------------------------
 */

void
buddy_free_block(struct double_buddy *db,
                 enum buddy_series series,
                 unsigned order,
                 uintptr_t address)
{
    while (order < MAX_ORDER) {
        uintptr_t buddy;
        struct free_block *other;
        struct buddy_list *list;

        buddy = buddy_address(
            db,
            series,
            order,
            address
        );

        list = get_list(db, series, order);

        other = list_find(list, buddy);

        if (!other)
            break;

        /*
         * Remove the buddy and form the parent block.
         */
        list_remove(list, other);

        if (buddy < address)
            address = buddy;

        order++;
    }

    list_insert(
        get_list(db, series, order),
        (struct free_block *)address
    );
}

/*
 * ----------------------------------------------------------------------
 * Public malloc/free interface.
 * ----------------------------------------------------------------------
 */

void *
buddy_malloc(size_t size)
{
    struct double_buddy *db;
    struct allocation *header;
    enum buddy_series series;
    unsigned order;
    size_t total;
    size_t block_size;
    void *block;

    if (size == 0)
        return NULL;

    /*
     * Header + requested payload.
     */
    if (size > SIZE_MAX - sizeof(*header))
        return NULL;

    total = sizeof(*header) + size;

    if (!choose_class(total, &series, &order))
        return NULL;

    /*
     * A production allocator would maintain a global/per-thread arena
     * pool rather than creating one here.
     */
    db = arena_create();

    if (!db)
        return NULL;

    block = buddy_alloc(db, series, order);

    if (!block)
        return NULL;

    block_size = series_size(series, order);

    header = block;

    header->magic = ALLOC_MAGIC;
    header->arena = db;
    header->block = (uintptr_t)block;
    header->block_size = block_size;
    header->series = series;
    header->order = order;
    header->mapping_size = ARENA_SIZE;

    return (void *)(header + 1);
}

void
buddy_free(void *ptr)
{
    struct allocation *header;
    struct double_buddy *db;

    if (!ptr)
        return;

    header = (struct allocation *)ptr - 1;

    if (header->magic != ALLOC_MAGIC)
        return;

    db = header->arena;

    if (!db || db->magic != ARENA_MAGIC)
        return;

    buddy_free_block(
        db,
        header->series,
        header->order,
        header->block
    );

    header->magic = 0;

    /*
     * This simplified implementation has one arena per allocation.
     * Therefore the arena cannot be unmapped immediately after free:
     * the free-list structures live inside it.
     *
     * A real implementation would maintain arenas independently and
     * reclaim an arena only when all of its blocks are free.
     */
}

/*
 * ----------------------------------------------------------------------
 * calloc/realloc convenience functions
 * ----------------------------------------------------------------------
 */

void *
buddy_calloc(size_t nmemb, size_t size)
{
    size_t total;
    void *ptr;

    if (nmemb != 0 && size > SIZE_MAX / nmemb)
        return NULL;

    total = nmemb * size;

    ptr = buddy_malloc(total);

    if (!ptr)
        return NULL;

    {
        unsigned char *p = ptr;

        for (size_t i = 0; i < total; i++)
            p[i] = 0;
    }

    return ptr;
}

void *
buddy_realloc(void *ptr, size_t size)
{
    struct allocation *header;
    size_t old_size;
    void *new_ptr;

    if (!ptr)
        return buddy_malloc(size);

    if (size == 0) {
        buddy_free(ptr);
        return NULL;
    }

    header = (struct allocation *)ptr - 1;

    if (header->magic != ALLOC_MAGIC)
        return NULL;

    old_size = header->block_size - sizeof(*header);

    /*
     * The existing block is already large enough.
     */
    if (size <= old_size)
        return ptr;

    new_ptr = buddy_malloc(size);

    if (!new_ptr)
        return NULL;

    {
        unsigned char *src = ptr;
        unsigned char *dst = new_ptr;

        for (size_t i = 0; i < old_size; i++)
            dst[i] = src[i];
    }

    buddy_free(ptr);

    return new_ptr;
}
