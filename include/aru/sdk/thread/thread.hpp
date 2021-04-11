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
 * @file thread.hpp
 * @brief 
 * @author wotsen (astralrovers@outlook.com)
 * @version 1.0.0
 * @date 2021-04-05
 * 
 * @copyright MIT
 * 
 */
#pragma once

#include <thread>
#include <atomic>
#include <chrono>
#include <string>
#include <stdexcept>
#include <string.h>
#include "../macros/defs.hpp"
#include "../lock/lock.hpp"
#include "thread_util.hpp"
#include "thread_routine.hpp"
#include "thread_pool.hpp"
#include "thread_object.hpp"
#include "thread_local_storage.hpp"

namespace aru  {

namespace sdk  {

typedef struct Thread_s Thread;

struct Thread_s {
    thread_t tid;
    thread_attr_t attr;
    char name[ARU_THREAD_NAME_LEN];
    enum lock_type type;
    union {
        spin_lock_t spin;
        mutex_lock_t mutex;
        sem_lock_t sem;
    } lock;
    mutex_cond_t cond;
    bool run;
    void *(*func)(Thread *, void *);
    void *arg;
};

void __thread_init(Thread *t, void *(*func)(Thread *, void *), void *arg, const thread_attr_t *attr, enum lock_type type);
void __thread_deinit(Thread *t);
static inline void *__thread_func(void *arg) {
    Thread *t = (Thread*)arg;
    if (!t->func) {
        return nullptr;
    }

    t->run = true;
    auto ret = t->func(t, t->arg);
    t->run = false;

    return ret;
}

Thread *thread_create(void *(*func)(Thread *, void *), void *arg, const thread_attr_t *attr, enum lock_type type=THREAD_LOCK_MUTEX);
static inline Thread *thread_create(void *(*func)(Thread *, void *), void *arg, enum lock_type type=THREAD_LOCK_MUTEX) {
    return thread_create(func, arg, nullptr, type);
}

static inline int thread_set_name(Thread *t, const char *name) {
    if (!t || !name) {
        return -1;
    }
    strncpy(t->name, name, ARU_MIN(ARU_THREAD_NAME_LEN, strlen(name)));
    return thread_set_name(t->tid, t->name);
}

static inline int thread_detach(Thread *t) {
    if (!t) {
        return -1;
    }

    return thread_detach(t->tid);
}

static inline int thread_join(Thread *t, void **ret=nullptr) {
    if (!t) {
        return -1;
    }
    return thread_join(t->tid, ret);
}

int thread_destroy(Thread *t);

static inline int thread_lock(Thread *t) {
    if (!t) {
        return -1;
    }

    switch (t->type) {
        case THREAD_LOCK_SPIN:
            return spin_lock(&t->lock.spin);
            break;
        case THREAD_LOCK_SEM:
            return sem_lock_wait(&t->lock.sem, -1);
            break;
        case THREAD_LOCK_MUTEX:
            return mutex_lock(&t->lock.mutex);
            break;
        default:
            break;
    }
    return -1;
}
static inline int thread_unlock(Thread *t) {
    if (!t) {
        return -1;
    }

    switch (t->type) {
        case THREAD_LOCK_SPIN:
            return spin_unlock(&t->lock.spin);
            break;
        case THREAD_LOCK_SEM:
            return sem_lock_signal(&t->lock.sem);
            break;
        case THREAD_LOCK_MUTEX:
            return mutex_unlock(&t->lock.mutex);
            break;
        default:
            break;
    }
    return 0;
}

static inline int thread_wait(Thread *t, int64_t ms) {
    if (!t) {
        return -1;
    }

    switch (t->type) {
        case THREAD_LOCK_COND:
            return mutex_cond_wait(&t->lock.mutex, &t->cond, ms);
            break;
        case THREAD_LOCK_SEM:
            return sem_lock_wait(&t->lock.sem, ms);
            break;
        default:
            break;
    }
    return 0;
}

static inline int thread_signal(Thread *t, bool all=false) {
    if (!t) {
        return -1;
    }

    switch (t->type) {
        case THREAD_LOCK_COND:
            if (all) {
                return mutex_cond_signal_all(&t->cond);
            } else {
                return mutex_cond_signal(&t->cond);
            }
            break;
        case THREAD_LOCK_SEM:
            return sem_lock_signal(&t->lock.sem);
            break;
        default:
            break;
    }
    return 0;
}

class AruThread {
public:
    AruThread(void *(*func)(Thread *, void *)) : AruThread(func, nullptr, nullptr) {}
    AruThread(void *(*func)(Thread *, void *), void *arg) : AruThread(func, arg, nullptr) {}
    AruThread(void *(*func)(Thread *, void *), enum lock_type type) : AruThread(func, nullptr, nullptr, type) {}
    AruThread(void *(*func)(Thread *, void *), thread_attr_t *attr) : AruThread(func, nullptr, attr) {}
    AruThread(void *(*func)(Thread *, void *), void *arg, thread_attr_t *attr, enum lock_type type=THREAD_LOCK_MUTEX) {
        if (!func) {
            throw std::invalid_argument("func is null");
        }
        __thread_init(&thread_, func, arg, attr, type);
    }
    AruThread(const AruThread &) = delete;
    AruThread &operator=(const AruThread &) = delete;
    virtual ~AruThread() {
        __thread_deinit(&thread_);
    }

    int start(const char *name=nullptr) {
        auto ret = thread_create(&thread_.tid, __thread_func, &thread_.attr, thread_.arg);
        if (ret != 0) {
            thread_.run = false;
            return -1;
        }
        if (name) {
            setname(name);
        }
        return 0;
    }

    thread_t tid(void) {
        return thread_.tid;
    }

    int setname(const char *name) {
        return thread_set_name(&thread_, name);
    }
    std::string name(void) {
        return std::string(thread_.name);
    }

    int detach(void) {
        return thread_detach(thread_.tid);
    }
    int join(void **ret) {
        return thread_join(thread_.tid, ret);
    }

    int yield(void) {
        return thread_yield();
    }

    int lock(void) {
        return thread_lock(&thread_);
    }
    int unlock(void) {
        return thread_unlock(&thread_);
    }

    int wait(int64_t ms=-1) {
        return thread_wait(&thread_, ms);
    }
    int signal(bool all=false) {
        return thread_signal(&thread_, all);
    }

    int alive(void) {
        return thread_alive(thread_.tid);
    }
    int cancel(void) {
        return thread_cancel(thread_.tid);
    }

    bool state(void) {
        return thread_.run;
    }
    int attribute(thread_attr_t *attr) {
        if (!attr) {
            return -1;
        }

        memcpy(attr, &thread_.attr, sizeof(*attr));
        return 0;
    }

private:
    Thread thread_;
};

} // namespace sdk

} // namespace aru
