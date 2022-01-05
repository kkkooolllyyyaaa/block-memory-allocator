//
// Created by Цыпандин Николай Петрович on 05.01.2022.
//
#include <stdint.h>
#include <stdlib.h>
#include "mem_debug.h"
#include "mem_internals.h"
#include "mem.h"
#include "tests.h"
#include "util.h"

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
    heap = heap_init(16384);
    if (heap == NULL) {
        err("Heap init is failed: memory mapping error\n");
    }
    head = (struct block_header *) heap;
    if (head != NULL) {
        debug_heap(stderr, heap);
        debug("Heap is successfully created: ok\n");
    } else {
        err("Heap init is failed: void* casting error\n");
    }
}

static void run_test_1() {
    debug(" --- TEST 1 --- \n\n");
    size_t len = 5, query = sizeof(uint64_t) * len;
    uint64_t *test_arr = _malloc(query);
    if (test_arr == NULL)
        err("Test 1 is failed: can't allocate memory\n");
    if (head->capacity.bytes != (query) || head->is_free)
        err("Test 1 is failed: wrong allocation\n");
    for (size_t i = 0; i < len; ++i) {
        test_arr[i] = i * i;
    }
    debug_heap(stderr, heap);
    debug("Test 1 is passed: ok\n");
}

static void run_test_2() {
    debug(" --- TEST 2 --- \n\n");
//    size_t query = 2000;
//    void *mem_second = _malloc(query);
//    void *mem_third = _malloc(query);
//    _free(mem);
//    free(mem_second);
//    free(mem_third);
}

static void run_test_3() {
    debug(" --- TEST 3 --- \n\n");

}

static void run_test_4() {
    debug(" --- TEST 4 --- \n\n");

}

void run_tests() {
    init();
    run_test_1();
    run_test_2();
    run_test_3();
    run_test_4();
}
