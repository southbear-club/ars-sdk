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
 * @file lock_guard.hpp
 * @brief 守护锁
 * @author wotsen (astralrovers@outlook.com)
 * @version 1.0.0
 * @date 2021-04-11
 * 
 * @copyright MIT
 * 
 */
#pragma once
#include "rwlock.hpp"

namespace ars {
    
namespace sdk {

/**
 * @brief 读锁守护锁
 * 
 */
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

/**
 * @brief 写锁守护锁
 * 
 */
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

} // namespace sdk

} // namespace ars
