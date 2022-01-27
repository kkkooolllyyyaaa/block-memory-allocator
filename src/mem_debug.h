//
// Created by Цыпандин Николай Петрович on 05.01.2022.
//

#ifndef ASSIGNMENT_MEMORY_ALLOCATOR_MEM_DEBUG_H
#define ASSIGNMENT_MEMORY_ALLOCATOR_MEM_DEBUG_H

#include <stdarg.h>
#include <stdio.h>

#include "mem.h"
#include "mem_internals.h"

void debug_block(struct block_header *b, const char *fmt, ...);

void debug(const char *fmt, ...);

#endif //ASSIGNMENT_MEMORY_ALLOCATOR_MEM_DEBUG_H
