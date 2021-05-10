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
 * @file kmp.cpp
 * @brief
 * @author  ()
 * @version 1.0.0
 * @date 2021-04-07
 *
 * @copyright MIT
 *
 */

// Knuth-Morris-Pratt Algorithm
// http://www.ics.uci.edu/~eppstein/161/960227.html

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ars/sdk/algorithm/algorithm.hpp"

namespace ars {

namespace sdk {

static void kmp_overlap(const char* pattern, int n, int* overlap) {
    int i, j;

    overlap[0] = 0;

    for (i = 0, j = 1; j < n; j++) {
        assert(i < n);
        if (pattern[j] == pattern[i]) {
            overlap[j] = ++i;
        } else {
            i = 0;
            overlap[j] = 0;
        }
    }
}

static const char* kmp_match(const char* s, const char* pattern, int n1, int n2, int* overlap) {
    int i, j;

    i = 0;
    j = 0;
    while (i < n1 && j < n2) {
        // assert(i+j >= 0 && i+j<n1);
        if (s[i] == pattern[j]) {
            ++j;
            ++i;
        } else {
            j = j > 0 ? overlap[j - 1] : 0;
            i += j > 0 ? 0 : 1;
        }
    }

    assert(i >= j);
    return j == n2 ? s + i - j : 0;
}

const char* kmp(const char* s, const char* pattern) {
    int n1, n2;
    int* overlap;
    const char* p;

    assert(pattern);
    n1 = strlen(s);
    n2 = strlen(pattern);
    overlap = (int*)malloc(sizeof(int) * (n2 + 1));
    if (!overlap) return nullptr;

    kmp_overlap(pattern, n2, overlap);
    p = kmp_match(s, pattern, n1, n2, overlap);

    free(overlap);
    return p;
}

}  // namespace sdk

}  // namespace ars
