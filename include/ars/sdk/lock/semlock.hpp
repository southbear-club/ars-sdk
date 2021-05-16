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
 * @file semlock.hpp
 * @brief 信号量
 * @author wotsen (astralrovers@outlook.com)
 * @version 1.0.0
 * @date 2021-05-16
 * 
 * @copyright MIT
 * 
 */
#pragma once
#include <semaphore.h>
#include <errno.h>
#include <time.h>
#include <stdint.h>

namespace ars {
    
namespace sdk {

typedef sem_t sem_lock_t;

// pshared, 0 : threads, 1 : proccess
static inline int sem_lock_init_ex(sem_lock_t *lock, int pshared, unsigned int value) {
    return ::sem_init(lock, pshared, value);
}

static inline int sem_lock_init(sem_lock_t *lock) {
    return ::sem_init(lock, 0, 0);
}

static inline int sem_lock_wait(sem_lock_t *lock, int64_t ms) {
    int ret = 0;
    if (!lock) {
        return -1;
    }
    if (ms < 0) {
        return ::sem_wait(lock);
    } else if (ms == 0) {
        return ::sem_trywait(lock);
    } else {
#ifndef __APPLE__
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        uint64_t ns = ts.tv_sec * 1000 * 1000 * 1000 + ts.tv_nsec;
        ns += ms * 1000 * 1000;
        ts.tv_sec = ns / (1000 * 1000 * 1000);
        ts.tv_nsec = ns % 1000 * 1000 * 1000;
        ret = ::sem_timedwait(lock, &ts);
        if (ret != 0) {
            switch (errno) {
            case EINVAL:
                // printf("The value of abs_timeout.tv_nsecs is less than 0, "
                //        "or greater than or equal to 1000 million.\n");
                break;
            case ETIMEDOUT:
                // printf("The call timed out before the semaphore could be locked.\n");
                break;
            }
        }
#else
        return ::sem_trywait(lock);
#endif
    }
    return ret;
}

static inline int sem_lock_trywait(sem_lock_t *lock)
{
    return sem_lock_wait(lock, 0);
}

static inline int sem_lock_signal(sem_lock_t *lock) {
    return ::sem_post(lock);
}

static inline int sem_lock_deinit(sem_lock_t *lock) {
    return ::sem_destroy(lock);
}

} // namespace sdk

} // namespace ars
