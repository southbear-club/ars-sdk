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
 * @file heap_sort.cpp
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

namespace aru {
    
namespace sdk {

#if 0
static void u32_swap(void *a, void *b, size_t size)
{
    uint32_t t = *(uint32_t *)a;
    *(uint32_t *)a = *(uint32_t *)b;
    *(uint32_t *)b = t;
}

static void generic_swap(void *a, void *b, size_t size)
{
    char t;
    char *_a = (char *)a;
    char *_b = (char *)b;
    do {
        t = *_a;
        *_a++ = *_b;
        *_b++ = t;
    } while (--size > 0);
}

static int generic_cmp(const void *a, const void *b, size_t size)
{
    const unsigned char *p = (const unsigned char *)a;
    const unsigned char *q = (const unsigned char *)b;
    for (; size--; p++, q++) {
        if (*p != *q) return *p - *q;
    }
    return 0;
}

/**
 * _heap_sort - sort an array of elements
 * @base: pointer to data to sort
 * @num: number of elements
 * @size: size of each element
 * @cmp_func: pointer to comparison function
 * @swap_func: pointer to swap function or NULL
 *
 * This function does a heapsort on the given array. You may provide a
 * swap_func function optimized to your element type.
 *
 * Sorting time is O(n log n) both on average and worst-case. While
 * qsort is about 20% faster on average, it suffers from exploitable
 * O(n*n) worst-case behavior and extra memory requirements that make
 * it less suitable for kernel use.
 */

static void _heap_sort(void *base, size_t num, size_t size,
          int (*cmp_func)(const void *, const void *, size_t),
          void (*swap_func)(void *, void *, size_t))
{
    /* pre-scale counters for performance */
    int i = (num/2 - 1) * size, n = num * size, c, r;

    if (!swap_func)
        swap_func = (size == 4 ? u32_swap : generic_swap);
    if (!cmp_func)
        cmp_func = generic_cmp;

    /* heapify */
    for ( ; i >= 0; i -= (int)size) {
        for (r = i; r * 2 + (int)size < n; r  = c) {
            c = r * 2 + (int)size;
            if (c < n - (int)size &&
                cmp_func(base + c, base + c + size, size) < 0)
                c += size;
            if (cmp_func(base + r, base + c, size) >= 0)
                break;
            swap_func(base + r, base + c, size);
        }
    }

    /* sort */
    for (i = n - (int)size; i > 0; i -= (int)size) {
        swap_func(base, base + i, size);
        for (r = 0; r * 2 + (int)size < i; r = c) {
            c = r * 2 + (int)size;
            if (c < i - (int)size &&
                cmp_func(base + c, base + c + size, size) < 0)
                c += (int)size;
            if (cmp_func(base + r, base + c, size) >= 0)
                break;
            swap_func(base + r, base + c, size);
        }
    }
}

void heap_sort(void *base, size_t num, size_t size, fp_cmp cmp) {
    _heap_sort(base, num, size, cmp, NULL);
}

#endif

} // namespace sdk

} // namespace aru
