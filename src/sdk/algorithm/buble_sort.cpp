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
 * @file buble_sort.cpp
 * @brief 
 * @author wotsen (astralrovers@outlook.com)
 * @version 1.0.0
 * @date 2021-04-10
 * 
 * @copyright MIT
 * 
 */
#include "sort_common.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

namespace ars {
    
namespace sdk {

#define BUBBLE_SORT(type, array, len) \
    do { \
        type *s = (type *)array; \
        type *e = s + len - 1; \
        type *p, *q; \
        for (q = e; q > s; q--) { \
            for (p = s; p < q; p++) { \
                if (p[0] > p[1]) { \
                    type t = p[0]; \
                    p[0] = p[1]; \
                    p[1] = t; \
                } \
            } \
        } \
    } while (0)

void bubble_sortf(float *array, size_t len)
{
    if (!array || len <=0) {
        return;
    }
    BUBBLE_SORT(float, array, len);
}

int bubble_sort(void *array, size_t num, size_t size, fp_cmp cmp)
{
    CHK_PARAMETERS(array, num, size, cmp);

    byte *s = (byte *)array;
    byte *e = s + size * (num - 1);

    byte *p, *q;
    for (q = e; q > s; q -= size) {
        for (p = s; p < q; p += size) {
            if (cmp(p, p + size, size) > 0) {
                byte_swap(p, p + size, size);
            }
        }
    }
    return 0;
}

} // namespace sdk

} // namespace ars
