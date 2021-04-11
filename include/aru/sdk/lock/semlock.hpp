#pragma once
#include <semaphore.h>
#include <errno.h>
#include <time.h>
#include <stdint.h>

namespace aru {
    
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
    struct timespec ts;
    if (!lock) {
        return -1;
    }
    if (ms < 0) {
        return ::sem_wait(lock);
    } else if (ms == 0) {
        return ::sem_trywait(lock);
    } else {
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

} // namespace aru
