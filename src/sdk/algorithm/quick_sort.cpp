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
 * @file quick_sort.cpp
 * @brief 
 * @author  ()
 * @version 1.0.0
 * @date 2021-04-10
 * 
 * @copyright MIT
 * 
 */
#include "sort_common.hpp"
#include <stdint.h>

namespace ars {
    
namespace sdk {

static void qsort_recu(byte *l, byte *r, size_t size, fp_cmp cmp)
{
    if (l < r) {
        byte *p = l;
        byte *q = r;

        do {
            while ((p < q) && (cmp(p, q, size) <= 0))
                q -= size;

            if (p != q) {
                byte_swap(p, q, size);
                p += size;
            }

            while ((p < q) && (cmp(p, q, size) <= 0))
                p += size;

            if (p != q) {
                byte_swap(q, p, size);
                q -= size;
            }
        } while (p < q);

        qsort_recu(l, p - size, size, cmp);
        qsort_recu(p + size, r, size, cmp);
    }
}

int quick_sort(void *array, size_t num, size_t size, fp_cmp cmp)
{
    CHK_PARAMETERS(array, num, size, cmp);
    qsort_recu((byte *)array, (byte *)array + size * (num - 1), size, cmp);
    return 0;
}

} // namespace sdk

} // namespace ars
