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
 * @file spinlock.cpp
 * @brief 
 * @author  ()
 * @version 1.0.0
 * @date 2021-04-10
 * 
 * @copyright MIT
 * 
 */
#include "ars/sdk/lock/spinlock.hpp"
#include "ars/sdk/atomic/atomic.hpp"
#include <pthread.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sched.h>

namespace ars {
    
namespace sdk {

#if ( __i386__ || __i386 || __amd64__ || __amd64 )
#define cpu_pause() __asm__ ("pause")
#else
#define cpu_pause()
#endif

int spin_lock_init(spin_lock_t *lock) {
#ifdef __APPLE__
    if (!lock) {
        return -1;
    }
    (*lock) = 0;
    return 0;
#else
    return ::pthread_spin_init(lock, PTHREAD_PROCESS_PRIVATE);
#endif
}

int spin_lock_destroy(spin_lock_t *lock) {
#ifdef __APPLE__
    if (!lock) {
        return -1;
    }
    (*lock) = 0;
    return 0;
#else
    return ::pthread_spin_destroy(lock);
#endif
}

int spin_lock(spin_lock_t *lock) {
#ifdef __APPLE__
    int spin = 2048;
    if (!lock) {
        return -1;
    }
    int value = 1;
    int i, n;
    auto ncpu = sysconf(_SC_NPROCESSORS_ONLN);
    for (;;) {
        if (*lock == 0 && atomic_compare_swap(lock, 0, value)) {
            return 0;
        }

        if (ncpu > 1) {
            for (n = 1; n < spin; n <<= 1) {
                for (i = 0; i < n; i++) {
                    cpu_pause();
                }
                if (*lock == 0 && atomic_compare_swap(lock, 0, value)) {
                    return 0;
                }
            }
        }
        sched_yield();
    }
    return 0;
#else
    return ::pthread_spin_lock(lock);
#endif
}

int spin_unlock(spin_lock_t *lock) {
#ifdef __APPLE__
    if (!lock) {
        return -1;
    }
    (*lock) = 0;

    return 0;
#else
    return ::pthread_spin_unlock(lock);
#endif
}

int spin_trylock(spin_lock_t *lock) {
#ifdef __APPLE__
    if (!lock) {
        return -1;
    }
    return (*lock) == 0 && atomic_compare_swap(lock, 0, 1);
#else
    return ::pthread_spin_trylock(lock);
#endif
}

} // namespace sdk

} // namespace ars
