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
 * @file rwlock.hpp
 * @brief 
 * @author wotsen (astralrovers@outlook.com)
 * @version 1.0.0
 * @date 2021-04-04
 * 
 * @copyright MIT
 * 
 */
#pragma once

#include <pthread.h>
#include <time.h>
#include <mutex>

#include "aru/sdk/patterns/singleton.hpp"

namespace aru {

namespace sdk {

#define ARU_RWLOCK_INIT PTHREAD_RWLOCK_INITIALIZER

typedef pthread_rwlock_t rw_lock_t;
typedef pthread_rwlockattr_t rw_lock_attr_t;

static inline int rwlock_init_ex(rw_lock_t *lock, rw_lock_attr_t *attr) {
    return ::pthread_rwlock_init(lock, attr);
}

static inline int rwlock_init(rw_lock_t *lock) {
    return ::pthread_rwlock_init(lock, nullptr);
}

static inline int rwlock_rdlock(rw_lock_t *lock) {
    return ::pthread_rwlock_rdlock(lock);
}

static inline int rwlock_tryrdlock(rw_lock_t *lock) {
    return ::pthread_rwlock_tryrdlock(lock);
}

static inline int rwlock_rdlock_wait(rw_lock_t *lock, const struct timespec *abs_tm) {
#ifdef __APPLE__
        if (abs_tm->tv_sec) {}
        return pthread_rwlock_tryrdlock(lock) == 0;
#else
        return pthread_rwlock_timedrdlock(lock, abs_tm) == 0;
#endif
}

static inline int rwlock_wrlock(rw_lock_t *lock) {
    return ::pthread_rwlock_wrlock(lock);
}

static inline int rwlock_trywrlock(rw_lock_t *lock) {
    return ::pthread_rwlock_trywrlock(lock);
}

static inline int rwlock_wrlock_wait(rw_lock_t *lock, const struct timespec *abs_tm) {
#ifdef __APPLE__
        if (abs_tm->tv_sec) {}
        return pthread_rwlock_trywrlock(lock) == 0;
#else
        return pthread_rwlock_timedwrlock(lock, abs_tm) == 0;
#endif
}

static inline int rwlock_unlock(rw_lock_t *lock) {
    return ::pthread_rwlock_unlock(lock);
}

static inline void rwlock_deinit(rw_lock_t *lock) {
    ::pthread_rwlock_destroy(lock);
}

class RwMutex {
public:
    RwMutex() { rwlock_init(&_mutex); }

    ~RwMutex() { rwlock_deinit(&_mutex); }

    void rlock() { rwlock_rdlock(&_mutex); }

    void wlock() { rwlock_wrlock(&_mutex); }

    bool try_rlock() { return rwlock_tryrdlock(&_mutex) == 0; }

    bool try_wlock() { return rwlock_trywrlock(&_mutex) == 0; }

    bool try_rlock_timeout(struct timespec& t) {
        return rwlock_rdlock_wait(&_mutex, &t) == 0;
    }

    bool try_wlock_timeout(struct timespec& t) {
        return rwlock_wrlock_wait(&_mutex, &t) == 0;
    }

    void unlock() { rwlock_unlock(&_mutex); }

    rw_lock_t* mutex() { return &_mutex; }

private:
    rw_lock_t _mutex;
    DISALLOW_COPY_AND_ASSIGN(RwMutex);
};

}  // namespace sdk

}  // namespace aru
