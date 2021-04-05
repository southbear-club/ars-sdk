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
 * @file defs.hpp
 * @brief 
 * @author wotsen (astralrovers@outlook.com)
 * @version 1.0.0
 * @date 2021-04-04
 * 
 * @copyright MIT
 * 
 */
#pragma once
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "platform.hpp"

#define ARU_ABS(n)  ((n) > 0 ? (n) : -(n))
#define ARU_NABS(n) ((n) < 0 ? (n) : -(n))
#define ARU_ARRAY_SIZE(a) (sizeof(a) / sizeof(*(a)))
#define ARU_BITSET(p, n) (*(p) |= (1u << (n)))
#define ARU_BITCLR(p, n) (*(p) &= ~(1u << (n)))
#define ARU_BITGET(i, n) ((i) & (1u << (n)))
#define ARU_CR      '\r'
#define ARU_LF      '\n'
#define ARU_CRLF    "\r\n"
#define ARU_FLOAT_PRECISION     1e-6
#define ARU_FLOAT_EQUAL_ZERO(f) (ARU_ABS(f) < ARU_FLOAT_PRECISION)
#define ARU_INFINITE    (uint32_t)-1
/*
ASCII:
[0, 0x20)    control-charaters
[0x20, 0x7F) printable-charaters

0x0A => LF
0x0D => CR
0x20 => SPACE
0x7F => DEL

[0x09, 0x0D] => \t\n\v\f\r
[0x30, 0x39] => 0~9
[0x41, 0x5A] => A~Z
[0x61, 0x7A] => a~z
*/
#define ARU_IS_ALPHA(c) (((c) >= 'a' && (c) <= 'z') || ((c) >= 'A' && (c) <= 'Z'))
#define ARU_IS_NUM(c)   ((c) >= '0' && (c) <= '9')
#define ARU_IS_ALPHANUM(c) (ARU_IS_ALPHA(c) || ARU_IS_NUM(c))
#define ARU_IS_CNTRL(c) ((c) >= 0 && (c) < 0x20)
#define ARU_IS_GRAPH(c) ((c) >= 0x20 && (c) < 0x7F)
#define ARU_IS_HEX(c)   (ARU_IS_NUM(c) || ((c) >= 'a' && (c) <= 'f') || ((c) >= 'A' && (c) <= 'F'))
#define ARU_IS_LOWER(c) (((c) >= 'a' && (c) <= 'z'))
#define ARU_IS_UPPER(c) (((c) >= 'A' && (c) <= 'Z'))
#define ARU_LOWER(c)    ((c) | 0x20)
#define ARU_UPPER(c)    ((c) & ~0x20)

// LD, LU, LLD, LLU for explicit conversion of integer
#define ARU_LD(v)   ((long)(v))
#define ARU_LU(v)   ((unsigned long)(v))
#define ARU_LLD(v)  ((long long)(v))
#define ARU_LLU(v)  ((unsigned long long)(v))

#define ARU_MAKEINT64(h, l)   ( ((int64_t)h) << 32 | (l & 0xffffffff) )
#define ARU_HIINT(n)          ( (int32_t)(((int64_t)n) >> 32) )
#define ARU_LOINT(n)          ( (int32_t)(n & 0xffffffff) )
#define ARU_MAKE_FOURCC(a, b, c, d) \
( ((uint32)d) | ( ((uint32)c) << 8 ) | ( ((uint32)b) << 16 ) | ( ((uint32)a) << 24 ) )
#define ARU_LIMIT(lower, v, upper) ((v) < (lower) ? (lower) : (v) > (upper) ? (upper) : (v))

#define ARU_STRINGIFY(x)    ARU_STRINGIFY_HELPER(x)
#define ARU_STRINGIFY_HELPER(x)     #x

#define ARU_STRINGCAT(x, y)  ARU_STRINGCAT_HELPER(x, y)
#define ARU_STRINGCAT_HELPER(x, y)  x##y

#define ARU_SAFE_FREE(p)    do {if (p) {free(p); (p) = NULL;}} while(0)
#define ARU_SAFE_DELETE(p)  do {if (p) {delete (p); (p) = NULL;}} while(0)
#define ARU_SAFE_DELETE_ARRAY(p) do {if (p) {delete[] (p); (p) = NULL;}} while(0)
#define ARU_SAFE_RELEASE(p) do {if (p) {(p)->release(); (p) = NULL;}} while(0)

#ifndef offsetof
#define offsetof(type, member) \
((size_t)(&((type*)0)->member))
#endif

#ifndef offsetofend
#define offsetofend(type, member) \
(offsetof(type, member) + sizeof(((type*)0)->member))
#endif

#ifndef container_of
#define container_of(ptr, type, member) \
((type*)((char*)(ptr) - offsetof(type, member)))
#endif

// ENDIAN
#define ARU_BIG_ENDIAN      4321
#define ARU_LITTLE_ENDIAN   1234
#define ARU_NET_ENDIAN      ARU_BIG_ENDIAN

// BYTE_ORDER
#if defined(ARU_ARCH_X86) || defined(ARU_ARCH_X86_64) || defined(ARU___ARMEL__)
#define ARU_BYTE_ORDER      ARU_LITTLE_ENDIAN
#elif defined(ARU___ARMEB__)
#define ARU_BYTE_ORDER      ARU_BIG_ENDIAN
#endif

#ifdef DEBUG
#define printd(...) printf(__VA_ARGS__)
#define printe(...) fprintf(stderr, __VA_ARGS__)
#else
#define printd(...)
#define printe(...)
#endif