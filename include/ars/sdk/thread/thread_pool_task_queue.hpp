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
 * @file thread_pool_task_queue.hpp
 * @brief 
 * @author wotsen (astralrovers@outlook.com)
 * @version 1.0.0
 * @date 2021-04-04
 * 
 * @copyright MIT
 * 
 */
#pragma once

#include <stddef.h>
#include <list>
#include <thread>
#include <mutex>
#include <condition_variable>

namespace ars {

namespace sdk {

/**
 * @brief 任务队列
 * 
 * @tparam T 队列类型
 */
template <typename T>
class ThreadPoolSyncTaskQueue {
public:
    ThreadPoolSyncTaskQueue(size_t size) : max_size_(size) {}
    virtual ~ThreadPoolSyncTaskQueue() { stop(); }

    // 入队
    virtual bool push(T&& obj) {
        std::unique_lock<std::mutex> lck(mutex_);
        // 等待队列非满才能入队
        not_full_.wait(lck, [this] { return stop_ || (count() < max_size_); });

        if (stop_) {
            return false;
        }

        queue_.push_back(std::forward<T>(obj));
        not_empty_.notify_one();

        return true;
    }

    // 出队
    virtual bool pop(T& t) {
        std::unique_lock<std::mutex> lck(mutex_);
        // 等待队列非空才能出队
        not_empty_.wait(lck, [this] { return stop_ || !queue_.empty(); });

        if (stop_) {
            return false;
        }

        t = queue_.front();
        queue_.pop_front();
        not_full_.notify_one();

        return true;
    }

    // 队列内容数
    virtual size_t count(void) { return queue_.size(); }

    virtual bool empty(void) { return queue_.empty(); }

    virtual bool full(void) { return queue_.size() >= max_size_; }

    // 停止队列
    virtual void stop(void) {
        {
            std::unique_lock<std::mutex> lck(mutex_);
            stop_ = true;
        }

        not_full_.notify_all();
        not_empty_.notify_all();
    }

private:
    std::list<T> queue_;                 ///< 任务队列
    std::mutex mutex_;                   ///< 队列锁
    std::condition_variable not_empty_;  ///< 非空条件变量
    std::condition_variable not_full_;   ///< 非满条件变量
    size_t max_size_;                    ///< 任务队列限制
    bool stop_;                          ///< 退出条件
};

} // namespace sdk

} // namespace ars