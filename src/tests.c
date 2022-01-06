//
// Created by Цыпандин Николай Петрович on 05.01.2022.
//

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include "mem_debug.h"
#include "mem_internals.h"
#include "mem.h"
#include "tests.h"
#include "util.h"

#define BLOCK_SIZE 2000
#define BIG_REGION (REGION_MIN_SIZE*3/2)
#define newline "\n"
#define TEST_OK(x, y) "TEST " x " is passed: " y newline
#define TEST_BAD(x, y) "TEST " x " is failed: " y newline

static struct block_header *head;
static void *heap;

extern inline struct block_header *get_header_by_addr(void *addr);

static void init() {
    debug(" --- TEST HEAP INIT ---" newline);
    heap = heap_init(REGION_MIN_SIZE);
    if (heap == NULL)
        err(TEST_BAD("HEAP INIT", "memory mapping error"));
    head = (struct block_header *) heap;
    if (head == NULL)
        err(TEST_BAD("HEAP INIT", "void* casting error"));
    debug(TEST_OK("HEAP INIT", "OK"));
    debug(newline);
}

static void run_test_1() {
    debug(" --- TEST 1 --- "newline);
    size_t len = 5, query = sizeof(uint64_t) * len;
    uint64_t *test_arr = _malloc(query, heap);

    if (test_arr == NULL)
        err(TEST_BAD("1", "can't allocate memory"));

    if (head->capacity.bytes != (query) || head->is_free)
        err(TEST_BAD("1", "wrong allocation"));

    for (size_t i = 0; i < len; ++i)
        test_arr[i] = i * i;
    if (test_arr[0] != 0 || test_arr[1] != 1 || test_arr[2] != 4 || test_arr[3] != 9 || test_arr[4] != 16)
        err(TEST_BAD("1", "can't use allocated memory"));

    debug(TEST_OK("1", "OK"));
    debug(newline);
}

static void run_test_2() {
    debug(" --- TEST 2 --- "newline);

    void *addr_1 = _malloc(BLOCK_SIZE, heap);
    void *addr_2 = _malloc(2 * BLOCK_SIZE, heap);
    void *addr_3 = _malloc(BLOCK_SIZE, heap);

    if (addr_1 == NULL || addr_2 == NULL || addr_3 == NULL)
        err(TEST_BAD("2", "can't allocate memory"));
    _free(addr_2);

    struct block_header *h1 = get_header_by_addr(addr_1),
            *h2 = get_header_by_addr(addr_2),
            *h3 = get_header_by_addr(addr_3);

    if (h1 == NULL || h2 == NULL || h3 == NULL)
        err(TEST_BAD("2", "can't get header by address"));
    if (h1->is_free || !h2->is_free || h3->is_free)
        err(TEST_BAD("2", "incorrect block freeing"));

    debug(TEST_OK("2", "OK"));
    debug(newline);
    debug_heap(stdout, heap);
    debug(newline);
}

static void run_test_3() {
    debug(" --- TEST 3 --- "newline);
    uint8_t *addr_1 = (uint8_t *) _malloc(BLOCK_SIZE / 2, heap);
    uint8_t *addr_2 = (uint8_t *) _malloc(BLOCK_SIZE / 2, heap);

    if (addr_1 == NULL || addr_2 == NULL)
        err(TEST_BAD("3", "can't allocate memory"));
    _free(addr_1);
    _free(addr_2);

    struct block_header *h1 = get_header_by_addr(addr_1),
            *h2 = get_header_by_addr(addr_2);

    if (h1 == NULL || h2 == NULL)
        err(TEST_BAD("3", "can't get header by address"));
    if (!h1->is_free || !h2->is_free)
        err(TEST_BAD("3", "incorrect block freeing"));

    debug(TEST_OK("3", "OK"));
    debug(newline);
}

static void run_test_4() {
    debug(" --- TEST 4 --- "newline);
    void *addr = _malloc(BIG_REGION, heap);
    if (addr == NULL)
        err(TEST_BAD("4", "can't allocate big amount of memory"));

    struct block_header *header = get_header_by_addr(addr);
    if (header == NULL)
        err(TEST_BAD("4", "can't get header by address"));
    if (header->is_free)
        err(TEST_BAD("4", "bad allocate, block is not free"));
    if (header->capacity.bytes != BIG_REGION)
        err(TEST_BAD("4", "memory isn't allocated"));

    debug(TEST_OK("4", "OK"));
    debug(newline);
}

static void run_test_5() {
    debug(" --- TEST 5 --- "newline);
    void *addr = _malloc(BIG_REGION, heap);
    if (addr == NULL)
        err(TEST_BAD("4", "can't allocate big amount of memory"));

    struct block_header *header = get_header_by_addr(addr);
    if (header == NULL)
        err(TEST_BAD("4", "can't get header by address"));
    if (header->is_free)
        err(TEST_BAD("4", "bad allocate, block is not free"));
    if (header->capacity.bytes != BIG_REGION)
        err(TEST_BAD("4", "memory isn't allocated"));

    debug(TEST_OK("4", "OK"));
    debug(newline);
}

void run_tests() {
    init();
    run_test_1();
    run_test_2();
    run_test_3();
    run_test_4();
    run_test_5();
}
