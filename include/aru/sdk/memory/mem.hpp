/**
 * Copyright © 2021 <wotsen>.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 * and associated documentation files (the “Software”), to deal in the Software without
 * restriction, including without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
 * 
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 * BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 * 
 * @file mem.hpp
 * @brief 
 * @author wotsen (astralrovers@outlook.com)
 * @version 1.0.0
 * @date 2021-04-04
 * 
 * @copyright MIT
 * 
 */
#pragma once
#include <stddef.h>
#include <stdlib.h>

namespace aru {

namespace sdk {

typedef struct {
    void *(*malloc)(size_t);
    void *(*realloc)(void *, size_t);
    void *(*calloc)(size_t, size_t);
    void (*free)(void*);
} memory_conf_t;

void aru_memory_init(const memory_conf_t &conf);

void *aru_malloc(size_t size);
void *aru_realloc(void *oldptr, size_t newsize, size_t oldsize);
void *aru_calloc(size_t nmemb, size_t size);
void *aru_zalloc(size_t size);
void aru_free(void *ptr);

long alloc_cnt(void);
long free_cnt(void);
void memroy_check(void);

static inline void memcheck_register(void) {
    atexit(memroy_check);
}

#define ARU_ALLOC(ptr, size) \
    do {\
        *(void**)&(ptr) = aru::sdk::aru_zalloc(size);\
    } while (0)

#define ARU_ALLOC_SIZEOF(ptr) ARU_ALLOC(ptr, sizeof(*(ptr)))

#define ARU_FREE(ptr) \
    do {\
        if (ptr) {\
            aru::sdk::aru_free(ptr);\
            ptr = NULL;\
        }\
    } while (0)

} // namespace sdk

} // namespace aru
