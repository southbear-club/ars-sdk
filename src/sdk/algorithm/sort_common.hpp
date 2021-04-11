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
 * @file sort_common.hpp
 * @brief 
 * @author  ()
 * @version 1.0.0
 * @date 2021-04-10
 * 
 * @copyright MIT
 * 
 */
#pragma once
#include "aru/sdk/algorithm/sort.hpp"

namespace aru {
    
namespace sdk {

typedef unsigned char byte;

#define CHK_PARAMETERS(array, num, size, cmp) \
    do { \
        if (!array || !size) { \
            return -1; \
        } \
        if (num < 2) { \
            return -1; \
        } \
        if (num >> 20) { \
            return -1; \
        } \
        if (!cmp) cmp = default_cmp; \
    } while (0)

static inline void byte_swap(byte *p, byte *q, size_t size)
{
    for (; size--; p++, q++) {
        byte t = *p;
        *p = *q;
        *q = t;
    }
}

static inline __attribute__((unused)) int default_cmp(const void *a, const void *b, size_t size)
{
    const byte *p = (const byte *)a;
    const byte *q = (const byte *)b;
    for (; size--; p++, q++) {
        if (*p != *q) return *p - *q;
    }
    return 0;
}

} // namespace sdk

} // namespace aru
