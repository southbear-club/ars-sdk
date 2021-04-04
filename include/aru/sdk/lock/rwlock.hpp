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

/**
 * @brief 锁
 *
 */
class ILock {
public:
    virtual ~ILock() {}

    // 上锁
    virtual bool lock(void) = 0;
    // 解锁
    virtual bool unlock(void) = 0;
    // 尝试锁
    virtual bool try_lock(time_t w = -1) = 0;
    // TODO:wait
    // TODO:notyfiy
};

class CppMtxLock : public ILock {
public:
    virtual ~CppMtxLock() {}

    virtual bool lock(void) override {
        mtx_.lock();
        return true;
    }

    virtual bool unlock(void) override {
        mtx_.unlock();
        return true;
    }

    virtual bool try_lock(time_t w = -1) override {
        if (w) {}
        return mtx_.try_lock();
    }

public:
    std::mutex mtx_;
};

class RwMutex {
public:
    RwMutex() { pthread_rwlock_init(&_mutex, nullptr); }

    ~RwMutex() { pthread_rwlock_destroy(&_mutex); }

    void rlock() { pthread_rwlock_rdlock(&_mutex); }

    void wlock() { pthread_rwlock_wrlock(&_mutex); }

    bool try_rlock() { return pthread_rwlock_tryrdlock(&_mutex) == 0; }

    bool try_wlock() { return pthread_rwlock_trywrlock(&_mutex) == 0; }

    bool try_rlock_timeout(struct timespec& t) {
#ifdef __APPLE__
        if (t.tv_sec) {}
        return pthread_rwlock_tryrdlock(&_mutex) == 0;
#else
        return pthread_rwlock_timedrdlock(&_mutex, &t) == 0;
#endif
    }

    bool try_wlock_timeout(struct timespec& t) {
#ifdef __APPLE__
        if (t.tv_sec) {}
        return pthread_rwlock_trywrlock(&_mutex) == 0;
#else
        return pthread_rwlock_timedwrlock(&_mutex, &t) == 0;
#endif
    }

    void unlock() { pthread_rwlock_unlock(&_mutex); }

    pthread_rwlock_t* mutex() { return &_mutex; }

private:
    pthread_rwlock_t _mutex;
    DISALLOW_COPY_AND_ASSIGN(RwMutex);
};

class RMutexGuard {
public:
    explicit RMutexGuard(RwMutex& lock) : _lock(lock) { _lock.rlock(); }
    explicit RMutexGuard(RwMutex* lock) : _lock(*lock) { _lock.rlock(); }

    ~RMutexGuard() { _lock.unlock(); }

    void lock() { _lock.rlock(); }
    void unlock() { _lock.unlock(); }

private:
    RwMutex& _lock;
    DISALLOW_COPY_AND_ASSIGN(RMutexGuard);
};

class WMutexGuard {
public:
    explicit WMutexGuard(RwMutex& lock) : _lock(lock) { _lock.wlock(); }
    explicit WMutexGuard(RwMutex* lock) : _lock(*lock) { _lock.wlock(); }

    ~WMutexGuard() { _lock.unlock(); }

    void lock() { _lock.wlock(); }
    void unlock() { _lock.unlock(); }

private:
    RwMutex& _lock;
    DISALLOW_COPY_AND_ASSIGN(WMutexGuard);
};

}  // namespace sdk

}  // namespace aru
