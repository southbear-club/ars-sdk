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
 * @file select_sort.cpp
 * @brief 
 * @author  ()
 * @version 1.0.0
 * @date 2021-04-10
 * 
 * @copyright MIT
 * 
 */
#include "sort_common.hpp"

namespace aru {
    
namespace sdk {

int select_sort(void *array, size_t num, size_t size, fp_cmp cmp)
{
    CHK_PARAMETERS(array, num, size, cmp);

    byte *s = (byte *)array;
    byte *e = s + (num - 1) * size;

    for (; s < e; s += size) {
        byte *first = s;
        byte *p = first + 1;
        for (; p <= e; p++) {
            if (cmp(p, first, size) < 0) {
                first = p;
            }
        }
        if (first != s) {
            byte_swap(first, s, size);
        }
    }

    return 0;
}

} // namespace sdk

} // namespace aru
