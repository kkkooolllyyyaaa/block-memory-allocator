
#define _DEFAULT_SOURCE

#include <assert.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "mem.h"
#include "mem_internals.h"
#include "util.h"

void debug_block(struct block_header *b, const char *fmt, ...);

void debug(const char *fmt, ...);

extern inline block_size size_from_capacity(block_capacity cap);

extern inline block_capacity capacity_from_size(block_size sz);

extern inline bool region_is_invalid(const struct region *r);

static bool block_is_big_enough(size_t query, struct block_header *block) {
    return block->capacity.bytes >= query;
}

static size_t pages_count(size_t mem) {
    return mem / getpagesize() + ((mem % getpagesize()) > 0);
}

static size_t round_pages(size_t mem) {
    return getpagesize() * pages_count(mem);
}

static void block_init(void *restrict addr, block_size block_sz, void *restrict next) {
    *((struct block_header *) addr) = (struct block_header) {
            .next = next,
            .capacity = capacity_from_size(block_sz),
            .is_free = true
    };
}

static size_t region_actual_size(size_t query) {
    return size_max(round_pages(query), REGION_MIN_SIZE);
}

static void *map_pages(void const *addr, size_t length, int additional_flags) {
    return mmap((void *) addr, length, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | additional_flags, -1, 0);
}

static bool is_bad_map(void const *addr) {
    return addr == NULL || addr == MAP_FAILED;
}

/*  аллоцировать регион памяти и инициализировать его блоком */
static struct region alloc_region(void const *addr, size_t query) {
    query = region_actual_size(query + offsetof(struct block_header, contents));
    void *reg_addr = map_pages(addr, query, MAP_FIXED_NOREPLACE);

    if (is_bad_map(reg_addr)) {
        reg_addr = map_pages(addr, query, 0);
        if (is_bad_map(reg_addr))
            return REGION_INVALID;
        const struct region new_region = {.addr = reg_addr, .size = query, false};
        block_init(reg_addr, (block_size) {query}, NULL);
        return new_region;
    }
    const struct region new_region = {.addr = reg_addr, .size = query, true};
    block_init(reg_addr, (block_size) {query}, NULL);
    return new_region;
}

static void *block_after(struct block_header const *block);

void *heap_init(size_t initial) {
    const struct region region = alloc_region(HEAP_START, initial);
    if (region_is_invalid(&region))
        return NULL;
    return region.addr;
}

#define BLOCK_MIN_CAPACITY 24

/*  --- Разделение блоков (если найденный свободный блок слишком большой )--- */

static bool block_splittable(struct block_header *restrict block, size_t query) {
    return block->is_free &&
           query + offsetof(struct block_header, contents) + BLOCK_MIN_CAPACITY <= block->capacity.bytes;
}

static void *split_block_addr(struct block_header *restrict block, block_size new_size) {
    return (void *) ((uint8_t *) block + new_size.bytes);
}

static bool split_if_too_big(struct block_header *block, size_t query) {
    if (!block_splittable(block, query))
        return false;

    const block_size new_size = {query + offsetof(struct block_header, contents)};
    const block_size next_size = {size_from_capacity(block->capacity).bytes - new_size.bytes};

    void *next_addr = split_block_addr(block, new_size);
    block_init(next_addr, next_size, block->next);
    block_init(block, new_size, next_addr);
    return true;
}


/*  --- Слияние соседних свободных блоков --- */

static void *block_after(struct block_header const *block) {
    return (void *) (block->contents + block->capacity.bytes);
}

static bool blocks_continuous(
        struct block_header const *fst,
        struct block_header const *snd) {
    return (void *) snd == block_after(fst);
}

static bool mergeable(struct block_header const *restrict fst, struct block_header const *restrict snd) {
    return fst->is_free && snd->is_free && blocks_continuous(fst, snd);
}

static bool try_merge_with_next(struct block_header *block) {
    if (!block->next)
        return false;
    if (!mergeable(block, block->next))
        return false;
    block->capacity.bytes += size_from_capacity(block->next->capacity).bytes;
    block->next = block->next->next;
    return true;
}


/*  --- ... ecли размера кучи хватает --- */

struct block_search_result {
    enum {
        BSR_FOUND_GOOD_BLOCK = 0, BSR_REACHED_END_NOT_FOUND, BSR_CORRUPTED
    } type;
    struct block_header *block;
};


static struct block_search_result find_good_or_last(struct block_header *restrict block, size_t sz) {
    if (!block)
        return (struct block_search_result) {.type = BSR_CORRUPTED, .block = NULL};

    struct block_header *cur = block;
    struct block_header *last = NULL;
    while (cur) {
        if (cur->is_free && block_is_big_enough(sz, cur))
            return (struct block_search_result) {.type = BSR_FOUND_GOOD_BLOCK, .block = cur};
        if (try_merge_with_next(cur))
            continue;

        last = cur;
        cur = cur->next;
    }
    return (struct block_search_result) {.type = BSR_REACHED_END_NOT_FOUND, .block = last};

}

/*  Попробовать выделить память в куче начиная с блока `block` не пытаясь расширить кучу
 Можно переиспользовать как только кучу расширили. */
static struct block_search_result try_memalloc_existing(size_t query, struct block_header *block) {
    query = size_max(query, BLOCK_MIN_CAPACITY);
    struct block_search_result res = find_good_or_last(block, query);

    if (res.type == BSR_FOUND_GOOD_BLOCK) {
        split_if_too_big(res.block, query);
    }
    return res;
}


static struct block_header *grow_heap(struct block_header *restrict last, size_t query) {
    const void * new_region_addr = block_after(last);
    const struct region new_region = alloc_region(new_region_addr, query);
    if (!region_is_invalid(&new_region)) {
        last->next = (struct block_header *) new_region.addr;
        if (try_merge_with_next(last)) {
            return last;
        }
        return (struct block_header *) new_region.addr;
    }
    return NULL;

}

/*  Реализует основную логику malloc и возвращает заголовок выделенного блока */
static struct block_header *memalloc(size_t query, struct block_header *heap_start) {
    const size_t actual_size = size_max(BLOCK_MIN_CAPACITY, query);
    struct block_search_result res = try_memalloc_existing(actual_size, heap_start);
    if (res.type == BSR_FOUND_GOOD_BLOCK) {
        res.block->is_free = false;
        return res.block;
    } else if (res.type == BSR_CORRUPTED)
        return NULL;
    else if (res.type == BSR_REACHED_END_NOT_FOUND)
        grow_heap(res.block, query);
    res = try_memalloc_existing(query, heap_start);
    if (res.type == BSR_REACHED_END_NOT_FOUND || res.type == BSR_CORRUPTED)
        return NULL;
    res.block->is_free = false;
    return res.block;
}

void *_malloc(size_t query, void *heap) {
    struct block_header *const addr = memalloc(query, heap);
    if (addr) return addr->contents;
    else return NULL;
}

static struct block_header *block_get_header(void *contents) {
    return (struct block_header *) (((uint8_t *) contents) - offsetof(struct block_header, contents));
}

void _free(void *mem) {
    if (!mem) return;
    struct block_header *header = block_get_header(mem);
    header->is_free = true;
}
