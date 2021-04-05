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
 * @file queue.hpp
 * @brief 
 * @author wotsen (astralrovers@outlook.com)
 * @version 1.0.0
 * @date 2021-04-04
 * 
 * @copyright MIT
 * 
 */
#pragma once

/*
 * queue
 * FIFO: push_back,pop_front
 * stack
 * LIFO: push_back,pop_back
 */

#include <assert.h> // for assert
#include <stddef.h> // for NULL
#include <stdlib.h> // for malloc,realloc,free
#include <string.h> // for memset,memmove
#include "../memory/mem.hpp"

#define ARU_QUEUE_INIT_SIZE     16

// #include <deque>
// typedef std::deque<type> qtype;
#define ARU_QUEUE_DECL(type, qtype) \
struct qtype {      \
    type*   ptr;    \
    size_t  size;   \
    size_t  maxsize;\
    size_t  _offset;\
};                  \
typedef struct qtype qtype;\
\
static inline type* qtype##_data(qtype* p) {\
    return p->ptr + p->_offset;\
}\
\
static inline int qtype##_size(qtype* p) {\
    return p->size;\
}\
\
static inline int qtype##_maxsize(qtype* p) {\
    return p->maxsize;\
}\
\
static inline int qtype##_empty(qtype* p) {\
    return p->size == 0;\
}\
\
static inline type* qtype##_front(qtype* p) {\
    return p->size == 0 ? NULL : p->ptr + p->_offset;\
}\
\
static inline type* qtype##_back(qtype* p) {\
    return p->size == 0 ? NULL : p->ptr + p->_offset + p->size - 1;\
}\
\
static inline void qtype##_init(qtype* p, int maxsize) {\
    p->_offset = 0;\
    p->size = 0;\
    p->maxsize = maxsize;\
    ARU_ALLOC(p->ptr, sizeof(type) * maxsize);\
}\
\
static inline void qtype##_clear(qtype* p) {\
    p->_offset = 0;\
    p->size = 0;\
    memset(p->ptr, 0, sizeof(type) * p->maxsize);\
}\
\
static inline void qtype##_cleanup(qtype* p) {\
    ARU_FREE(p->ptr);\
    p->_offset = p->size = p->maxsize = 0;\
}\
\
static inline void qtype##_resize(qtype* p, int maxsize) {\
    if (maxsize == 0) maxsize = QUEUE_INIT_SIZE;\
    p->ptr = (type*)aru::sdk::aru_realloc(p->ptr, sizeof(type) * maxsize, sizeof(type) * p->maxsize);\
    p->maxsize = maxsize;\
}\
\
static inline void qtype##_double_resize(qtype* p) {\
    qtype##_resize(p, p->maxsize * 2);\
}\
\
static inline void qtype##_push_back(qtype* p, type* elem) {\
    if (p->size == p->maxsize) {\
        qtype##_double_resize(p);\
    }\
    else if (p->_offset + p->size == p->maxsize) {\
        memmove(p->ptr, p->ptr + p->_offset, sizeof(type) * p->size);\
        p->_offset = 0;\
    }\
    p->ptr[p->_offset + p->size] = *elem;\
    p->size++;\
}\
static inline void qtype##_pop_front(qtype* p) {\
    assert(p->size > 0);\
    p->size--;\
    if (++p->_offset == p->maxsize) p->_offset = 0;\
}\
\
static inline void qtype##_pop_back(qtype* p) {\
    assert(p->size > 0);\
    p->size--;\
}\
