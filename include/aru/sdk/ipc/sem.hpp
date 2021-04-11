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
 * @file sem.hpp
 * @brief 
 * @author  ()
 * @version 1.0.0
 * @date 2021-04-10
 * 
 * @copyright MIT
 * 
 */
#pragma once
#include <stddef.h>
#include <fcntl.h>
#include <unistd.h>
#include <semaphore.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>

namespace aru {
    
namespace sdk {

namespace ipc {

typedef ::sem_t sem_t;

static inline sem_t *sem_open(const char *name, int oflag, mode_t mode, unsigned int value) {
    return ::sem_open(name, oflag, mode, value);
}

static inline int sem_close(sem_t *sem) {
    return ::sem_close(sem);
}

static inline int sem_unlink(const char *name) {
    return ::sem_unlink(name);
}

static inline int sem_getvalue(sem_t *sem, int *sval) {
    return ::sem_getvalue(sem, sval);
}

static inline int sem_pop(sem_t *sem) {
    return ::sem_wait(sem);
}

static inline int sem_trypop(sem_t *sem) {
    return ::sem_trywait(sem);
}

static inline int sem_pop_wait(sem_t *sem, const struct timespec *abs_tm) {
    return ::sem_timedwait(sem, abs_tm);
}

static inline int sem_post(sem_t *sem) {
    return ::sem_post(sem);
}

} // namespace ipc

} // namespace sdk

} // namespace aru
