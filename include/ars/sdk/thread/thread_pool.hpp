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
 * @file thread_pool.hpp
 * @brief 
 * @author wotsen (astralrovers@outlook.com)
 * @version 1.0.0
 * @date 2021-04-04
 * 
 * @copyright MIT
 * 
 */
#pragma once

#include <atomic>
#include <future>
#include <memory>
#include <vector>
#include <thread>
#include <queue>
#include <functional>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <utility>
#include "thread_pool_task_queue.hpp"

namespace ars {

namespace sdk {

/**
 * @brief 任务接口类型
 * 
 */
typedef std::function<void(void)> thread_task_t;

/**
 * @brief 微内核线程池
 * 
 */
class ThreadTaskPool {
public:
    ThreadTaskPool(size_t task_limit = 100,
                          int thread_cnt = std::thread::hardware_concurrency())
        : queue_(task_limit), running_(false) {
        running_ = true;
        for (int i = 0; i < thread_cnt; i++) {
            threads_.push_back(
                std::make_shared<std::thread>([this] { run(); }));
        }
    }

    virtual ~ThreadTaskPool() { stop(); }

    virtual void run() {
        while (running_) {
            thread_task_t t = nullptr;
            auto ret = queue_.pop(t);

            if (!ret || !t || !running_) {
                return;
            }

            t();
        }
    }

    virtual void stop() {
        std::call_once(flag_, [this] { _stop(); });  // 多线程只调用一次
    }

    // XXX:这里不做不定参数的接口，外部传入时可以自行绑定
    virtual void add_task(const thread_task_t &task) {
        queue_.push([task]() { task(); });
    }

private:
    void _stop(void) {
        queue_.stop();
        running_ = false;

        for (auto thread : threads_) {
            if (thread) {
                thread->join();
            }
        }

        threads_.clear();
    }

private:
    std::list<std::shared_ptr<std::thread>> threads_;  ///< 线程队列
    ThreadPoolSyncTaskQueue<thread_task_t> queue_;          ///< 线程任务队列
    std::atomic_bool running_;  ///< 线程池运行状态
    std::once_flag flag_;       ///< 标记
    std::mutex mutex_;          ///< 线程池锁
};

// 实现2
class ThreadPool {
public:
    using Task = std::function<void()>;

    ThreadPool(int size = std::thread::hardware_concurrency())
        : pool_size(size), idle_num(size), status(STOP) {
    }

    ~ThreadPool() {
        stop();
    }

    int start() {
        if (status == ThreadPool::Status::STOP) {
            status = ThreadPool::Status::RUNNING;
            for (int i = 0; i < pool_size; ++i) {
                workers.emplace_back(std::thread([this]{
                    while (status != ThreadPool::Status::STOP) {
                        while (status == ThreadPool::Status::PAUSE) {
                            std::this_thread::yield();
                        }

                        Task task;
                        {
                            std::unique_lock<std::mutex> locker(_mutex);
                            _cond.wait(locker, [this]{
                                return status == ThreadPool::Status::STOP || !tasks.empty();
                            });

                            if (status == ThreadPool::Status::STOP) return;

                            if (!tasks.empty()) {
                                --idle_num;
                                task = std::move(tasks.front());
                                tasks.pop();
                            }
                        }

                        task();
                        ++idle_num;
                    }
                }));
            }
        }
        return 0;
    }

    int stop() {
        if (status != ThreadPool::Status::STOP) {
            status = ThreadPool::Status::STOP;
            _cond.notify_all();
            for (auto& worker : workers) {
                worker.join();
            }
        }
        return 0;
    }

    int pause() {
        if (status == ThreadPool::Status::RUNNING) {
            status = ThreadPool::Status::PAUSE;
        }
        return 0;
    }

    int resume() {
        if (status == ThreadPool::Status::PAUSE) {
            status = ThreadPool::Status::RUNNING;
        }
        return 0;
    }

    int wait() {
        while (1) {
            if (status == ThreadPool::Status::STOP || (tasks.empty() && idle_num == pool_size)) {
                break;
            }
            std::this_thread::yield();
        }
        return 0;
    }

    // return a future, calling future.get() will wait task done and return RetType.
    // commit(fn, args...)
    // commit(std::bind(&Class::mem_fn, &obj))
    // commit(std::mem_fn(&Class::mem_fn, &obj))
    template<class Fn, class... Args>
    auto commit(Fn&& fn, Args&&... args) -> std::future<decltype(fn(args...))> {
        using RetType = decltype(fn(args...));
        auto task = std::make_shared<std::packaged_task<RetType()> >(
            std::bind(std::forward<Fn>(fn), std::forward<Args>(args)...));
        std::future<RetType> future = task->get_future();
        {
            std::lock_guard<std::mutex> locker(_mutex);
            tasks.emplace([task]{
                (*task)();
            });
        }

        _cond.notify_one();
        return future;
    }

public:
    enum Status {
        STOP,
        RUNNING,
        PAUSE,
    };
    int                 pool_size;
    std::atomic<int>    idle_num;
    std::atomic<ThreadPool::Status> status;
    std::vector<std::thread>    workers;
    std::queue<Task>            tasks;

protected:
    std::mutex              _mutex;
    std::condition_variable _cond;
};

} // namespace sdk

} // namespace ars
