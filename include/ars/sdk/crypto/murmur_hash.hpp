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
 * @file murmur_hash.hpp
 * @brief 
 * @author wotsen (astralrovers@outlook.com)
 * @version 1.0.0
 * @date 2021-04-05
 * 
 * @copyright MIT
 * 
 */

/*
 * Murmurhash from http://sites.google.com/site/murmurhash.
 * Written by Austin Appleby, and is placed to the public domain.
 * For business purposes, Murmurhash is under the MIT license.
 */

#pragma once

#include "../macros/platform.hpp"
#include <stdint.h>
#include <stddef.h>
#include <string.h>

namespace ars {
    
namespace sdk {

uint32_t murmur_hash32(const void* data, size_t len, uint32_t seed);
uint64_t murmur_hash64(const void* data, size_t len, uint64_t seed);

// murmur2 64 bit hash
inline uint64_t hash64(const void* s, size_t n) {
    return murmur_hash64(s, n, 0);
}

inline uint64_t hash64(const char* s) {
    return hash64(s, strlen(s));
}

template<typename S>
inline uint64_t hash64(const S& s) {
    return hash64(s.data(), s.size());
}

#if ARS_64BIT
// use the lower 32 bit of murmur_hash64 on 64 bit platform
inline uint32_t hash32(const void* s, size_t n) {
    return (uint32_t) hash64(s, n);
}

inline size_t murmur_hash(const void* s, size_t n) {
    return murmur_hash64(s, n, 0);
}
#else
// use murmur_hash32 on 32 bit platform
inline uint32_t hash32(const void* s, size_t n) {
    return murmur_hash32(s, n, 0);
}

inline size_t murmur_hash(const void* s, size_t n) {
    return murmur_hash32(s, n, 0);
}
#endif

inline uint32_t hash32(const char* s) {
    return hash32(s, strlen(s));
}

template<typename S>
inline uint32_t hash32(const S& s) {
    return hash32(s.data(), s.size());
}

inline size_t murmur_hash(const char* s) {
    return murmur_hash(s, strlen(s));
}

template<typename S>
inline size_t murmur_hash(const S& s) {
    return murmur_hash(s.data(), s.size());
}

} // namespace sdk

} // namespace ars
