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
 * @file atomic.hpp
 * @brief 原子操作
 * @author 
 * @version 1.0.0
 * @date 2021-04-10
 * 
 * @copyright MIT
 * 
 */
#pragma once
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>

namespace aru {
    
namespace sdk {

// clang or gcc 4.7+
#if defined(__clang__) || (defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 7)))

template <typename T>
inline T atomic_inc(T* p) {
    return __atomic_add_fetch(p, 1, __ATOMIC_SEQ_CST);
}

template <typename T>
inline T atomic_dec(T* p) {
    return __atomic_sub_fetch(p, 1, __ATOMIC_SEQ_CST);
}

template <typename T, typename V>
inline T atomic_add(T* p, V v) {
    return __atomic_add_fetch(p, v, __ATOMIC_SEQ_CST);
}

template <typename T, typename V>
inline T atomic_sub(T* p, V v) {
    return __atomic_sub_fetch(p, v, __ATOMIC_SEQ_CST);
}

template <typename T>
inline T atomic_fetch_inc(T* p) {
    return __atomic_fetch_add(p, 1, __ATOMIC_SEQ_CST);
}

template <typename T>
inline T atomic_fetch_dec(T* p) {
    return __atomic_fetch_sub(p, 1, __ATOMIC_SEQ_CST);
}

template <typename T, typename V>
inline T atomic_fetch_add(T* p, V v) {
    return __atomic_fetch_add(p, v, __ATOMIC_SEQ_CST);
}

template <typename T, typename V>
inline T atomic_fetch_sub(T* p, V v) {
    return __atomic_fetch_sub(p, v, __ATOMIC_SEQ_CST);
}

template <typename T, typename V>
inline T atomic_or(T* p, V v) {
    return __atomic_or_fetch (p, v, __ATOMIC_SEQ_CST);
}

template <typename T, typename V>
inline T atomic_and(T* p, V v) {
    return __atomic_and_fetch(p, v, __ATOMIC_SEQ_CST);
}

template <typename T, typename V>
inline T atomic_xor(T* p, V v) {
    return __atomic_xor_fetch(p, v, __ATOMIC_SEQ_CST);
}

template <typename T, typename V>
inline T atomic_fetch_or(T* p, V v) {
    return __atomic_fetch_or(p, v, __ATOMIC_SEQ_CST);
}

template <typename T, typename V>
inline T atomic_fetch_and(T* p, V v) {
    return __atomic_fetch_and(p, v, __ATOMIC_SEQ_CST);
}

template <typename T, typename V>
inline T atomic_fetch_xor(T* p, V v) {
    return __atomic_fetch_xor(p, v, __ATOMIC_SEQ_CST);
}

template <typename T, typename V>
inline T atomic_swap(T* p, V v) {
    return __atomic_exchange_n(p, (T)v, __ATOMIC_SEQ_CST);
}

template <typename T, typename O, typename V>
inline T atomic_compare_swap(T* p, O o, V v) {
    T x = (T) o;
    __atomic_compare_exchange_n(p, &x, (T)v, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    return x;
}

template <typename T>
inline T atomic_get(T* p) {
    return __atomic_load_n(p, __ATOMIC_SEQ_CST);
}

template <typename T, typename V>
inline void atomic_set(T* p, V v) {
    __atomic_store_n(p, (T)v, __ATOMIC_SEQ_CST);
}

template <typename T>
inline void atomic_reset(T* p) {
    __atomic_store_n(p, (T)0, __ATOMIC_SEQ_CST);
}

// gcc 4.1+
#elif defined(__GNUC__) && (__GNUC__ == 4 && __GNUC_MINOR__ >= 1)

template <typename T>
inline T atomic_inc(T* p) {
    return __sync_add_and_fetch(p, 1);
}

template <typename T>
inline T atomic_dec(T* p) {
    return __sync_sub_and_fetch(p, 1);
}

template <typename T, typename V>
inline T atomic_add(T* p, V v) {
    return __sync_add_and_fetch(p, v);
}

template <typename T, typename V>
inline T atomic_sub(T* p, V v) {
    return __sync_sub_and_fetch(p, v);
}

template <typename T>
inline T atomic_fetch_inc(T* p) {
    return __sync_fetch_and_add(p, 1);
}

template <typename T>
inline T atomic_fetch_dec(T* p) {
    return __sync_fetch_and_sub(p, 1);
}

template <typename T, typename V>
inline T atomic_fetch_add(T* p, V v) {
    return __sync_fetch_and_add(p, v);
}

template <typename T, typename V>
inline T atomic_fetch_sub(T* p, V v) {
    return __sync_fetch_and_sub(p, v);
}

template <typename T, typename V>
inline T atomic_or(T* p, V v) {
    return __sync_or_and_fetch(p, v);
}

template <typename T, typename V>
inline T atomic_and(T* p, V v) {
    return __sync_and_and_fetch(p, v);
}

template <typename T, typename V>
inline T atomic_xor(T* p, V v) {
    return __sync_xor_and_fetch(p, v);
}

template <typename T, typename V>
inline T atomic_fetch_or(T* p, V v) {
    return __sync_fetch_and_or(p, v);
}

template <typename T, typename V>
inline T atomic_fetch_and(T* p, V v) {
    return __sync_fetch_and_and(p, v);
}

template <typename T, typename V>
inline T atomic_fetch_xor(T* p, V v) {
    return __sync_fetch_and_xor(p, v);
}

template <typename T, typename V>
inline T atomic_swap(T* p, V v) {
    return __sync_lock_test_and_set(p, (T)v);  // acquire barrier
}

template <typename T, typename O, typename V>
inline T atomic_compare_swap(T* p, O o, V v) {
    return __sync_val_compare_and_swap(p, (T)o, (T)v);
}

template <typename T>
inline bool atomic_bool_compare_swap(T* p, T o, T v) {
    return __sync_bool_compare_and_swap(p, o, v);
}

template <typename T>
inline T atomic_get(T* p) {
    return atomic_fetch_or(p, 0);
}

//     |
//     v   ^
//---------|-----  release barrier
//
//---------|-----  acquire barrier
//     ^   v
//     |

template <typename T, typename V>
inline void atomic_set(T* p, V v) {
    __sync_lock_test_and_set(p, v); // acquire barrier
}

template <typename T>
inline void atomic_reset(T* p) {
    __sync_lock_release(p);         // release barrier
}

#else
#error "clang or gcc 4.1+ required"
#endif

} // namespace sdk

} // namespace aru
