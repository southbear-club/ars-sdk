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
 * @file thread.cpp
 * @brief 
 * @author  ()
 * @version 1.0.0
 * @date 2021-04-11
 * 
 * @copyright MIT
 * 
 */
#include "aru/sdk/thread/thread.hpp"
#include "aru/sdk/memory/mem.hpp"

namespace aru {
    
namespace sdk {
    
void __thread_init(Thread *t, void *(*func)(Thread *, void *), void *arg, const thread_attr_t *attr, enum lock_type type) {
    memset(t, 0, sizeof(*t));
    t->type = type;

    if (attr) {
        memcpy(&t->attr, attr, sizeof(*attr));
    } else {
        if (0 != thread_attr_init(&t->attr)) {
            throw std::runtime_error("init thread attr error");
        }
    }

    switch (type) {
        case THREAD_LOCK_SPIN:
            if (0 != spin_lock_init(&t->lock.spin)) {
                thread_attr_destroy(&t->attr);
                throw std::runtime_error("spin lock init error");
            }
            break;
        case THREAD_LOCK_SEM:
            if (0 != sem_lock_init(&t->lock.sem)) {
                thread_attr_destroy(&t->attr);
                throw std::runtime_error("sem lock init error");
            }
            break;
        case THREAD_LOCK_MUTEX:
            if (0 != mutex_lock_init(&t->lock.mutex)) {
                thread_attr_destroy(&t->attr);
                throw std::runtime_error("mutex lock init error");
            }
            break;
        case THREAD_LOCK_COND:
            if (0 != mutex_lock_init(&t->lock.mutex)) {
                thread_attr_destroy(&t->attr);
                throw std::runtime_error("mutex lock init error");
            }
            if (0 != mutex_cond_init(&t->cond)) {
                thread_attr_destroy(&t->attr);
                throw std::runtime_error("cond init error");
            }
            break;
        default:
            break;
    }
    t->arg = arg;
    t->func = func;
    t->run = false;
    t->tid = 0;
}

Thread *thread_create(void *(*func)(Thread *, void *), void *arg, const thread_attr_t *attr, enum lock_type type) {
    if (!func) {
        return nullptr;
    }
    Thread *t = (Thread*)aru_calloc(1, sizeof(Thread));
    if (!t) {
        return nullptr;
    }

    try {
        __thread_init(t, func, arg, attr, type);
    } catch (const std::runtime_error &) {
        aru_free(t);
        return nullptr;
    }

    auto ret = thread_create(&t->tid, __thread_func, &t->attr, t->arg);
    if (ret != 0) {
        thread_attr_destroy(&t->attr);
        aru_free(t);

        return nullptr;
    }

    return t;
}

void __thread_deinit(Thread *t) {
    switch (t->type) {
        case THREAD_LOCK_SPIN:
            break;
        case THREAD_LOCK_SEM:
            sem_lock_deinit(&t->lock.sem);
            break;
        case THREAD_LOCK_MUTEX:
            mutex_lock_deinit(&t->lock.mutex);
            break;
        case THREAD_LOCK_COND:
            mutex_cond_signal_all(&t->cond);
            mutex_lock_deinit(&t->lock.mutex);
            mutex_cond_deinit(&t->cond);
            break;
        default:
            break;
    }
    thread_attr_destroy(&t->attr);
}

int thread_destroy(Thread *t) {
    if (!t) {
        return -1;
    }

    __thread_deinit(t);
    aru_free(t);

    return 0;
}

} // namespace sdk

} // namespace aru
