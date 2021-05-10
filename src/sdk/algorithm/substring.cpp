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
 * @file substring.cpp
 * @brief
 * @author  ()
 * @version 1.0.0
 * @date 2021-04-07
 *
 * @copyright MIT
 *
 */
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ars/sdk/algorithm/algorithm.hpp"

namespace ars {

namespace sdk {

#define MAX(a, b) ((a) > (b) ? (a) : (b));
#define LEN(i, j) lcs_length(m, n1, n2, i, j)

static int lcs_length(const int* m, int n1, int n2, int i, int j) {
    return (i < 0 || j < 0) ? 0 : m[i * n2 + j];
}

static int lcs_alogrithm(const char* s1, const char* s2, int n1, int n2, int* m) {
    int i, j;
    int lcs, idx;

    lcs = 0;
    idx = 0;

    for (i = 0; i < n1; ++i) {
        for (j = 0; j < n2; ++j) {
            if (s1[i] == s2[j]) {
                m[i * n2 + j] = LEN(i - 1, j - 1) + 1;

                // longest
                if (m[i * n2 + j] > lcs) {
                    lcs = m[i * n2 + j];
                    idx = i * n2 + j;
                }

            } else {
                m[i * n2 + j] = 0;
            }
        }
    }

    return idx;
}

static void lcs_substring(const char* s1, const char* s2, int n1, int n2, const int* m, int idx,
                          char* sub) {
    int i, j;
    int lcs;

    for (i = idx / n2, j = idx % n2; i >= 0 && j > 0; --i, --j) {
        lcs = LEN(i, j);
        if (lcs <= 0) break;

        assert(LEN(i, j) > LEN(i - 1, j - 1));
        sub[lcs - 1] = s1[i];
    }
}

#ifdef DEBUG
static void lcs_print(const char* s1, const char* s2, int n1, int n2, int* m) {
    int i, j;

    for (i = 0; i < n2; i++) printf("\t%c", s2[i]);
    printf("\n");

    for (i = 0; i < n1; i++) {
        printf("%c\t", s1[i]);
        for (j = 0; j < n2; j++) {
            printf("%d\t", LEN(i, j));
        }
        printf("\n");
    }
}
#endif

/// longest common substring
///@param[in] s1 string source
///@param[in] s2 string source
///@param[out] sub common substring
///@param[in] len substring buffer size(bytes)
///@return <0-error, 0-ok, >0=need more buffer
int strsubstring(const char* s1, const char* s2, char* sub, int len) {
    int* m;
    int n1, n2;
    int idx;

    n1 = strlen(s1);
    n2 = strlen(s2);
    if (0 == n1 || 0 == n2) return 0;

    m = (int*)malloc(n1 * n2 * sizeof(int));
    if (!m) return -ENOMEM;

    idx = lcs_alogrithm(s1, s2, n1, n2, m);

#ifdef DEBUG
    lcs_print(s1, s2, n1, n2, m);
#endif

    if (len < m[idx]) {
        free(m);
        return m[idx];
    }
    sub[m[idx]] = 0;
    lcs_substring(s1, s2, n1, n2, m, idx, sub);

    free(m);
    return 0;
}

}  // namespace sdk

}  // namespace ars
