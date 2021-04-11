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
 * @file spinlock.hpp
 * @brief 
 * @author  ()
 * @version 1.0.0
 * @date 2021-04-10
 * 
 * @copyright MIT
 * 
 */
#pragma once
#include <pthread.h>

namespace aru {
    
namespace sdk {

// #define ARU_SPIN_LOCK_INIT 0

typedef pthread_spinlock_t spin_lock_t;

static inline int spin_lock_init_ex(spin_lock_t *lock, int pshared) {
    return ::pthread_spin_init(lock, pshared);
}

static inline int spin_lock_init(spin_lock_t *lock) {
    return ::pthread_spin_init(lock, PTHREAD_PROCESS_PRIVATE);
}

static inline int spin_lock_destroy(spin_lock_t *lock) {
    return ::pthread_spin_destroy(lock);
}

static inline int spin_lock(spin_lock_t *lock) {
    return ::pthread_spin_lock(lock);
}

static inline int spin_unlock(spin_lock_t *lock) {
    return ::pthread_spin_unlock(lock);
}

static inline int spin_trylock(spin_lock_t *lock) {
    return ::pthread_spin_trylock(lock);
}

} // namespace sdk

} // namespace aru
