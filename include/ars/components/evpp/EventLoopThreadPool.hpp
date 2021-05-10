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
 * @file EventLoopThreadPool.hpp
 * @brief 
 * @author wotsen (astralrovers@outlook.com)
 * @version 1.0.0
 * @date 2021-04-18
 * 
 * @copyright MIT
 * 
 */
#pragma once
#include "EventLoopThread.hpp"

namespace ars {
    
namespace evpp {

class EventLoopThreadPool : public Status {
public:
    EventLoopThreadPool(int thread_num = std::thread::hardware_concurrency()) {
        setStatus(kInitializing);
        thread_num_ = thread_num;
        next_loop_idx_ = 0;
        setStatus(kInitialized);
    }

    ~EventLoopThreadPool() {
        stop();
        join();
    }

    int threadNum() {
        return thread_num_;
    }

    void setThreadNum(int num) {
        thread_num_ = num;
    }

    EventLoopPtr nextLoop() {
        if (loop_threads_.empty()) return NULL;
        return loop_threads_[++next_loop_idx_ % loop_threads_.size()]->loop();
    }

    EventLoopPtr loop(int idx = -1) {
        if (idx >= 0 && (unsigned int)idx < loop_threads_.size()) {
            return loop_threads_[idx]->loop();
        }
        return nextLoop();
    }

    sdk::event::loop_t* hloop(int idx = -1) {
        EventLoopPtr ptr = loop(idx);
        return ptr ? ptr->loop() : NULL;
    }

    // @param wait_threads_started: if ture this method will block until all loop_threads started.
    // @param pre: This functor will be executed when loop_thread started.
    // @param post:This Functor will be executed when loop_thread stopped.
    void start(bool wait_threads_started = false,
               std::function<void(const EventLoopPtr&)> pre = NULL,
               std::function<void(const EventLoopPtr&)> post = NULL) {
        setStatus(kStarting);

        if (thread_num_ == 0) {
            setStatus(kRunning);
            return;
        }

        std::shared_ptr<std::atomic<int>> started_cnt(new std::atomic<int>(0));
        std::shared_ptr<std::atomic<int>> exited_cnt(new std::atomic<int>(0));

        for (int i = 0; i < thread_num_; ++i) {
            EventLoopThreadPtr loop_thread(new EventLoopThread);
            EventLoopPtr loop = loop_thread->loop();
            loop_thread->start(false,
                [this, started_cnt, pre, &loop]() {
                    if (++(*started_cnt) == thread_num_) {
                        setStatus(kRunning);
                    }
                    if (pre) pre(loop);
                    return 0;
                },
                [this, exited_cnt, post, &loop]() {
                    if (post) post(loop);
                    if (++(*exited_cnt) == thread_num_) {
                        setStatus(kStopped);
                    }
                    return 0;
                }
            );
            loop_threads_.push_back(loop_thread);
        }

        if (wait_threads_started) {
            while (status() < kRunning) {
                sdk::msdelay(1);
            }
        }
    }

    // @param wait_threads_started: if ture this method will block until all loop_threads stopped.
    void stop(bool wait_threads_stopped = false) {
        setStatus(kStopping);

        for (auto& loop_thread : loop_threads_) {
            loop_thread->stop(false);
        }

        if (wait_threads_stopped) {
            while (!isStopped()) {
                sdk::msdelay(1);
            }
        }
    }

    // @brief join all loop_threads
    // @note  destructor will join loop_threads if you forget to call this method.
    void join() {
        for (auto& loop_thread : loop_threads_) {
            loop_thread->join();
        }
    }

private:
    int                                         thread_num_;
    std::vector<EventLoopThreadPtr>             loop_threads_;
    std::atomic<unsigned int>                   next_loop_idx_;
};

} // namespace evpp

} // namespace ars
