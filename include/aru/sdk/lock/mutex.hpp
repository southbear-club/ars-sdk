
#pragma once
#include <pthread.h>
#include <stdint.h>
#include <time.h>
#include <errno.h>

namespace aru {
    
namespace sdk {

#define ARU_MTXLOCK_INIT PTHREAD_MUTEX_INITIALIZER
#define ARU_MTXCOND_INIT PTHREAD_COND_INITIALIZER

typedef pthread_mutex_t mutex_lock_t;
typedef pthread_mutexattr_t mutex_attr_t;
typedef pthread_cond_t mutex_cond_t;
typedef pthread_condattr_t mutex_cond_attr_t;

static inline int mutex_init_ex(mutex_lock_t *lock, mutex_attr_t *attr) {
    return ::pthread_mutex_init(lock, attr);
}

static inline int mutex_lock_init(mutex_lock_t *lock) {
    return mutex_init_ex(lock, nullptr);
}

static inline int mutex_lock(mutex_lock_t *lock) {
    return ::pthread_mutex_lock(lock);
}

static inline int mutex_trylock(mutex_lock_t *lock) {
    return ::pthread_mutex_trylock(lock);
}

static inline int mutex_lock_wait(mutex_lock_t *lock, const struct timespec *abs_tm) {
#ifdef __APPLE__
    if (abs_tm) {}
    return ::pthread_mutex_trylock(lock);
#else
    return ::pthread_mutex_timedlock(lock, abs_tm);
#endif
}

static inline int mutex_unlock(mutex_lock_t *lock) {
    return ::pthread_mutex_unlock(lock);
}

static inline int mutex_lock_deinit(mutex_lock_t *lock) {
    return ::pthread_mutex_destroy(lock);
}

static inline int mutex_cond_init_ex(mutex_cond_t *cond, const mutex_cond_attr_t *attr) {
    return ::pthread_cond_init(cond, attr);
}

static inline int mutex_cond_init(mutex_cond_t *cond) {
    return ::pthread_cond_init(cond, nullptr);
}

static inline int mutex_cond_wait(mutex_lock_t *mutex, mutex_cond_t *cond, int64_t ms) {
    int ret = 0;
    int retry = 3;
    struct timespec ts;
    if (!cond || !mutex) {
        return -1;
    }
    if (ms < 0) {
        //never return an error code
        pthread_cond_wait(cond, mutex);
    } else {
        clock_gettime(CLOCK_REALTIME, &ts);
        uint64_t ns = ts.tv_sec * 1000 * 1000 * 1000 + ts.tv_nsec;
        ns += ms * 1000 * 1000;
        ts.tv_sec = ns / (1000 * 1000 * 1000);
        ts.tv_nsec = ns % 1000 * 1000 * 1000;
wait:
        ret = pthread_cond_timedwait(cond, mutex, &ts);
        if (ret != 0) {
            switch (ret) {
            case ETIMEDOUT:
                // printf("the condition variable was not signaled "
                //        "until the timeout specified by abstime.\n");
                break;
            case EINTR:
                // printf("pthread_cond_timedwait was interrupted by a signal.\n");
                if (--retry != 0) {
                    goto wait;
                }
                break;
            default:
                // printf("pthread_cond_timedwait error:%s.\n", strerror(ret));
                break;
            }
        }
    }

    return ret;
}

static inline int mutex_cond_broadcast(mutex_cond_t *cond) {
    return ::pthread_cond_broadcast(cond);
}

static inline int mutex_cond_signal(mutex_cond_t *cond) {
    return ::pthread_cond_signal(cond);
}

static inline int mutex_cond_signal_all(mutex_cond_t *cond) {
    return ::pthread_cond_broadcast(cond);
}

static inline int mutex_cond_deinit(mutex_cond_t *cond) {
    return ::pthread_cond_destroy(cond);
}

} // namespace sdk

} // namespace aru
