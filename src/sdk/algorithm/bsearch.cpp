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
 * @file bsearch.cpp
 * @brief 
 * @author wotsen (astralrovers@outlook.com)
 * @version 1.0.0
 * @date 2021-04-05
 * 
 * @copyright MIT
 * 
 */
#include "aru/sdk/algorithm/bserach.hpp"

namespace aru {
    
namespace sdk {

/*
void* bsearch(const void* key, const void* arr, size_t num, size_t size,
				int (*cmp)(const void* key, const void* elt))
{
	int result;
	void* ptr;
	size_t start, end, mid;

	start = 0;
	end = start + num;

	while (start < end)
	{
		mid = (start + end) / 2;
		ptr = (char*)arr + mid * size;

		result = cmp(key, ptr);
		if (result < 0)
			end = mid;
		else if (result > 0)
			start = mid + 1;
		else
			return ptr;
	}

	return NULL;
}
*/

/// @param[out] pos key position
/// @return 0-find, other-not found, pos is insert position
int bsearch2(const void* key, const void* arr, const void** pos, size_t num, size_t size,
	int(*cmp)(const void* key, const void* elt))
{
	int result;
	const void* ptr;
	size_t start, end, mid;

	result = -1;
	start = 0;
	end = start + num;
	ptr = arr;

	while (start < end)
	{
		mid = (start + end) / 2;
		ptr = (char*)arr + mid * size;

		result = cmp(key, ptr);
		if (result < 0)
			end = mid;
		else if (result > 0)
			start = mid + 1;
		else
			break;
	}

	if (pos) *pos = result > 0 ? ((char*)ptr + size) : ptr;
	return result;
}

#if defined(_DEBUG) || defined(DEBUG)
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <time.h>
static int bsearch_int_compare(const void* key, const void* elt)
{
	return *(const int*)key - *(const int*)elt;
}

void bsearch_test(void)
{
	size_t i, j, num;
	int arr[10000], v, *p;

	srand((unsigned int)time(NULL));
	for (num = i = 0; i < sizeof(arr)/sizeof(arr[0]); i++)
	{
		v = rand();
		if (0 == bsearch2(&v, arr, (const void**)&p, num, sizeof(int), bsearch_int_compare))
		{
			assert(v == *p);
		}
		else
		{
			memmove(p + 1, p, (num - (p - arr)) * sizeof(int));
			*p = v;
			++num;

			for (j = 1; j < num; j++)
			{
				assert(arr[j - 1] < arr[j]);
			}
		}
	}

	printf("bsearch test ok\n");
}
#endif

} // namespace sdk

} // namespace aru
