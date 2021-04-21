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
 * @file EventLoopThread.hpp
 * @brief 
 * @author wotsen (astralrovers@outlook.com)
 * @version 1.0.0
 * @date 2021-04-18
 * 
 * @copyright MIT
 * 
 */
#pragma once
#include <thread>
#include "aru/sdk/time/time.hpp"
#include "EventLoop.hpp"

namespace aru {
    
namespace evpp {

class EventLoopThread : public Status {
public:
    // Return 0 means OK, other failed.
    typedef std::function<int()> Functor;

    EventLoopThread(EventLoopPtr loop = NULL) {
        setStatus(kInitializing);
        if (loop) {
            loop_ = loop;
        } else {
            loop_.reset(new EventLoop);
        }
        setStatus(kInitialized);
    }

    ~EventLoopThread() {
        stop();
        join();
    }

    EventLoopPtr loop() {
        return loop_;
    }

    sdk::event::loop_t* hloop() {
        return loop_->loop();
    }

    bool isRunning() {
        return loop_->isRunning();
    }

    // @param wait_thread_started: if ture this method will block until loop_thread started.
    // @param pre: This functor will be executed when loop_thread started.
    // @param post:This Functor will be executed when loop_thread stopped.
    void start(bool wait_thread_started = true,
               Functor pre = Functor(),
               Functor post = Functor()) {
        setStatus(kStarting);

        assert(thread_.get() == NULL);
        thread_.reset(new std::thread(&EventLoopThread::loop_thread, this, pre, post));

        if (wait_thread_started) {
            while (loop_->status() < kRunning) {
                sdk::msdelay(1);
            }
        }
    }

    // @param wait_thread_started: if ture this method will block until loop_thread stopped.
    void stop(bool wait_thread_stopped = false) {
        if (status() >= kStopping) return;
        setStatus(kStopping);

        loop_->stop();

        if (wait_thread_stopped) {
            while (!isStopped()) {
                sdk::msdelay(1);
            }
        }
    }

    // @brief join loop_thread
    // @note  destructor will join loop_thread if you forget to call this method.
    void join() {
        if (thread_ && thread_->joinable()) {
            thread_->join();
            thread_ = NULL;
        }
    }

private:
    void loop_thread(const Functor& pre, const Functor& post) {
        // hlogi("EventLoopThread started, tid=%ld", hv_gettid());
        setStatus(kStarted);

        if (pre) {
            loop_->queueInLoop([this, pre]{
                if (pre() != 0) {
                    loop_->stop();
                }
            });
        }

        loop_->run();
        assert(loop_->isStopped());

        if (post) {
            post();
        }

        setStatus(kStopped);
        // hlogi("EventLoopThread stopped, tid=%ld", hv_gettid());
    }

private:
    EventLoopPtr                 loop_;
    std::shared_ptr<std::thread> thread_;
};

typedef std::shared_ptr<EventLoopThread> EventLoopThreadPtr;

} // namespace evpp

} // namespace aru
