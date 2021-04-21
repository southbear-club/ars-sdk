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
 * @file EventLoop.hpp
 * @brief 
 * @author wotsen (astralrovers@outlook.com)
 * @version 1.0.0
 * @date 2021-04-18
 * 
 * @copyright MIT
 * 
 */
#pragma once
#include <functional>
#include <queue>
#include <map>
#include <mutex>
#include <assert.h>

#include "Event.hpp"
#include "Status.hpp"
#include "aru/sdk/thread/thread_local_storage.hpp"
#include "aru/sdk/thread/thread.hpp"
#include "aru/sdk/event/loop.hpp"

namespace aru {
    
namespace evpp {

class EventLoop : public Status {
public:

    typedef std::function<void()> Functor;

    // New an EventLoop using an existing hloop_t object,
    // so we can embed an EventLoop object into the old application based on hloop.
    // NOTE: Be careful to deal with destroy of hloop_t.
    EventLoop(sdk::event::loop_t* loop = NULL) {
        setStatus(kInitializing);
        if (loop) {
            loop_ = loop;
        } else {
            loop_ = sdk::event::loop_new(ARU_LOOP_FLAG_AUTO_FREE);
        }
        setStatus(kInitialized);
    }

    ~EventLoop() {
        stop();
    }

    sdk::event::loop_t* loop() {
        return loop_;
    }

    // @brief Run loop forever
    void run() {
        if (loop_ == NULL) return;
        sdk::ThreadLocalStorage::set(sdk::ThreadLocalStorage::EVENT_LOOP, this);
        setStatus(kRunning);
        sdk::event::loop_run(loop_);
        setStatus(kStopped);
    }

    void stop() {
        if (loop_ == NULL) return;
        setStatus(kStopping);
        sdk::event::loop_stop(loop_);
        loop_ = NULL;
    }

    void pause() {
        if (loop_ == NULL) return;
        if (isRunning()) {
            sdk::event::loop_pause(loop_);
            setStatus(kPause);
        }
    }

    void resume() {
        if (loop_ == NULL) return;
        if (isPause()) {
            sdk::event::loop_resume(loop_);
            setStatus(kRunning);
        }
    }

    // Timer interfaces: setTimer, killTimer, resetTimer
    TimerID setTimer(int timeout_ms, TimerCallback cb, int repeat = INFINITE) {
        sdk::event::timer_t* htimer = sdk::event::timer_add(loop_, onTimer, timeout_ms, repeat);

        Timer timer(htimer, cb, repeat);
        aru_event_set_userdata(htimer, this);

        TimerID timerID = aru_event_id(htimer);

        mutex_.lock();
        timers[timerID] = timer;
        mutex_.unlock();
        return timerID;
    }

    // alias javascript setTimeout, setInterval
    TimerID setTimeout(int timeout_ms, TimerCallback cb) {
        return setTimer(timeout_ms, cb, 1);
    }
    TimerID setInterval(int interval_ms, TimerCallback cb) {
        return setTimer(interval_ms, cb, INFINITE);
    }

    void killTimer(TimerID timerID) {
        std::lock_guard<std::mutex> locker(mutex_);
        auto iter = timers.find(timerID);
        if (iter != timers.end()) {
            Timer& timer = iter->second;
            sdk::event::timer_del(timer.timer);
            timers.erase(iter);
        }
    }

    void resetTimer(TimerID timerID) {
        std::lock_guard<std::mutex> locker(mutex_);
        auto iter = timers.find(timerID);
        if (iter != timers.end()) {
            Timer& timer = iter->second;
            sdk::event::timer_reset(timer.timer);
            if (timer.repeat == 0) {
                timer.repeat = 1;
            }
        }
    }

    long tid() {
        if (loop_ == NULL) sdk::gettid();
        return sdk::event::loop_tid(loop_);
    }

    bool isInLoopThread() {
        return (long)sdk::gettid() == sdk::event::loop_tid(loop_);
    }

    void assertInLoopThread() {
        assert(isInLoopThread());
    }

    void runInLoop(Functor fn) {
        if (isInLoopThread()) {
            if (fn) fn();
        } else {
            queueInLoop(fn);
        }
    }

    void queueInLoop(Functor fn) {
        postEvent([fn](Event* ev) {
            if (fn) fn();
        });
    }

    void postEvent(EventCallback cb) {
        if (loop_ == NULL) return;

        EventPtr ev(new Event(cb));
        aru_event_set_userdata(&ev->event, this);
        ev->event.cb = onCustomEvent;

        mutex_.lock();
        customEvents.push(ev);
        mutex_.unlock();

        sdk::event::loop_post_event(loop_, &ev->event);
    }

private:
    static void onTimer(sdk::event::timer_t* htimer) {
        EventLoop* loop = (EventLoop*)aru_event_userdata(htimer);

        TimerID timerID = aru_event_id(htimer);
        TimerCallback cb = NULL;

        loop->mutex_.lock();
        auto iter = loop->timers.find(timerID);
        if (iter != loop->timers.end()) {
            Timer& timer = iter->second;
            cb = timer.cb;
            --timer.repeat;
        }
        loop->mutex_.unlock();

        if (cb) cb(timerID);

        // NOTE: refind iterator, because iterator may be invalid
        // if the timer-related interface is called in the callback function above.
        loop->mutex_.lock();
        iter = loop->timers.find(timerID);
        if (iter != loop->timers.end()) {
            Timer& timer = iter->second;
            if (timer.repeat == 0) {
                // htimer_t alloc and free by hloop, but timers[timerID] managed by EventLoop.
                loop->timers.erase(iter);
            }
        }
        loop->mutex_.unlock();
    }

    static void onCustomEvent(sdk::event::event_t* hev) {
        EventLoop* loop = (EventLoop*)aru_event_userdata(hev);

        loop->mutex_.lock();
        EventPtr ev = loop->customEvents.front();
        loop->customEvents.pop();
        loop->mutex_.unlock();

        if (ev && ev->cb) ev->cb(ev.get());
    }

private:
    sdk::event::loop_t*                    loop_;
    std::mutex                  mutex_;
    std::queue<EventPtr>        customEvents;   // GUAREDE_BY(mutex_)
    std::map<TimerID, Timer>    timers;         // GUAREDE_BY(mutex_)
};

typedef std::shared_ptr<EventLoop> EventLoopPtr;

// ThreadLocalStorage
static inline EventLoop* tlsEventLoop() {
    return (EventLoop*)sdk::ThreadLocalStorage::get(sdk::ThreadLocalStorage::EVENT_LOOP);
}

static inline TimerID setTimer(int timeout_ms, TimerCallback cb, int repeat = INFINITE) {
    EventLoop* loop = tlsEventLoop();
    if (loop == NULL) return INVALID_TIMER_ID;
    return loop->setTimer(timeout_ms, cb, repeat);
}

static inline void killTimer(TimerID timerID) {
    EventLoop* loop = tlsEventLoop();
    if (loop == NULL) return;
    loop->killTimer(timerID);
}

static inline void resetTimer(TimerID timerID) {
    EventLoop* loop = tlsEventLoop();
    if (loop == NULL) return;
    loop->resetTimer(timerID);
}

static inline TimerID setTimeout(int timeout_ms, TimerCallback cb) {
    return setTimer(timeout_ms, cb, 1);
}

static inline TimerID setInterval(int interval_ms, TimerCallback cb) {
    return setTimer(interval_ms, cb, INFINITE);
}

} // namespace evpp

} // namespace aru
