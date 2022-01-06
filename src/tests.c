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
#define HEAP_SIZE 16000
#define newline "\n"
#define TEST_OK(x, y) "TEST " x " is passed: " y newline
#define TEST_BAD(x, y) "TEST " x " is failed: " y newline

static struct block_header *head;
static void *heap;

//static size_t get_size() {
//    struct block_header *cur = head;
//    size_t cnt = 0;
//    while (cur) {
//        cur = cur->next;
//        ++cnt;
//    }
//    return cnt;
//}

static void init() {
    debug(" --- TEST HEAP INIT ---" newline);
    heap = heap_init(HEAP_SIZE);
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

    debug(newline);
}

static void run_test_3() {
    debug(" --- TEST 3 --- "newline);

    debug(newline);
}

static void run_test_4() {
    debug(" --- TEST 4 --- "newline);

    debug(newline);
}

void run_tests() {
    init();
    run_test_1();
    run_test_2();
    run_test_3();
    run_test_4();
}
