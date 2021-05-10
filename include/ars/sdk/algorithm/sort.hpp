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
 * @file sort.hpp
 * @brief 
 * @author  ()
 * @version 1.0.0
 * @date 2021-04-10
 * 
 * @copyright MIT
 * 
 */
#include <stddef.h>

namespace ars {
    
namespace sdk {

typedef int (*fp_cmp)(const void *a, const void *b, size_t size);

/**
 * @brief 堆排序
 * 
 * @param base 数据
 * @param num 数据个数
 * @param size 每个数据的长度
 * @param cmp 比较函数
 */
void heap_sort(void *base, size_t num, size_t size, fp_cmp cmp);
int bubble_sort(void *array, size_t num, size_t size, fp_cmp cmp);
int quick_sort(void *array, size_t num, size_t size, fp_cmp cmp);
int select_sort(void *array, size_t num, size_t size, fp_cmp cmp);

} // namespace sdk

} // namespace ars
