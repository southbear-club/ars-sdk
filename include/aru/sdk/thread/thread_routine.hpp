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
 * @file thread_routine.hpp
 * @brief 
 * @author  ()
 * @version 1.0.0
 * @date 2021-04-07
 * 
 * @copyright MIT
 * 
 */
#pragma once

#include <thread>
#include <atomic>
#include <chrono>

namespace aru {
    
namespace sdk {

typedef void *(*thread_routine)(void*);
typedef void* thread_ret_t;

/************************************************
 * Thread
 * Status: STOP,RUNNING,PAUSE
 * Control: start,stop,pause,resume
 * first-level virtual: doTask
 * second-level virtual: run
************************************************/
class ThreadRoutine {
public:
    enum Status {
        STOP,
        RUNNING,
        PAUSE,
    };

    enum SleepPolicy {
        YIELD,
        SLEEP_FOR,
        SLEEP_UNTIL,
        NO_SLEEP,
    };

    ThreadRoutine() {
        status = STOP;
        status_changed = false;
        dotask_cnt = 0;
        sleep_policy = YIELD;
        sleep_ms = 0;
    }

    virtual ~ThreadRoutine() {}

    void setStatus(Status stat) {
        status_changed = true;
        status = stat;
    }

    void setSleepPolicy(SleepPolicy policy, uint32_t ms = 0) {
        sleep_policy = policy;
        sleep_ms = ms;
        setStatus(status);
    }

    virtual int start() {
        if (status == STOP) {
            thread = std::thread([this] {
                if (!doPrepare()) return;
                setStatus(RUNNING);
                run();
                setStatus(STOP);
                if (!doFinish()) return;
            });
        }
        return 0;
    }

    virtual int stop() {
        if (status != STOP) {
            setStatus(STOP);
        }
        if (thread.joinable()) {
            thread.join();  // wait thread exit
        }
        return 0;
    }

    virtual int pause() {
        if (status == RUNNING) {
            setStatus(PAUSE);
        }
        return 0;
    }

    virtual int resume() {
        if (status == PAUSE) {
            setStatus(RUNNING);
        }
        return 0;
    }

    virtual void run() {
        while (status != STOP) {
            while (status == PAUSE) {
                std::this_thread::yield();
            }

            doTask();
            ++dotask_cnt;

            ThreadRoutine::sleep();
        }
    }

    virtual bool doPrepare() {return true;}
    virtual void doTask() {}
    virtual bool doFinish() {return true;}

    std::thread thread;
    std::atomic<Status> status;
    uint32_t dotask_cnt;
protected:
    void sleep() {
        switch (sleep_policy) {
        case YIELD:
            std::this_thread::yield();
            break;
        case SLEEP_FOR:
            std::this_thread::sleep_for(std::chrono::milliseconds(sleep_ms));
            break;
        case SLEEP_UNTIL: {
            if (status_changed) {
                status_changed = false;
                base_tp = std::chrono::system_clock::now();
            }
            base_tp += std::chrono::milliseconds(sleep_ms);
            std::this_thread::sleep_until(base_tp);
        }
            break;
        default:    // donothing, go all out.
            break;
        }
    }

    SleepPolicy sleep_policy;
    uint32_t    sleep_ms;
    // for SLEEP_UNTIL
    std::atomic<bool> status_changed;
    std::chrono::system_clock::time_point base_tp;
};

} // namespace sdk

} // namespace aru
