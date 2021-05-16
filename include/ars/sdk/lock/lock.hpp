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
 * @file lock.hpp
 * @brief 锁
 * @author wotsen (astralrovers@outlook.com)
 * @version 1.0.0
 * @date 2021-04-11
 * 
 * @copyright MIT
 * 
 */
#pragma once
#include "mutex.hpp"
#include "rwlock.hpp"
#include "semlock.hpp"
#include "spinlock.hpp"

namespace ars {

namespace sdk {

/// 锁类型
enum lock_type {
    THREAD_LOCK_SPIN = 0,
    THREAD_LOCK_MUTEX,
    THREAD_LOCK_COND,
    THREAD_LOCK_RW,
    THREAD_LOCK_SEM,
};

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
};

} // namespace sdk

} // namespace ars
