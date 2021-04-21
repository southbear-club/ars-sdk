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
 * @file Status.hpp
 * @brief 
 * @author wotsen (astralrovers@outlook.com)
 * @version 1.0.0
 * @date 2021-04-18
 * 
 * @copyright MIT
 * 
 */
#pragma once
#include <atomic>

namespace aru {
    
namespace evpp {

class Status {
public:
    enum KStatus {
        kNull           = 0,
        kInitializing   = 1,
        kInitialized    = 2,
        kStarting       = 3,
        kStarted        = 4,
        kRunning        = 5,
        kPause          = 6,
        kStopping       = 7,
        kStopped        = 8,
        kDestroyed      = 9,
    };

    Status() {
        status_ = kNull;
    }
    ~Status() {
        status_ = kDestroyed;
    }

    KStatus status() {
        return status_;
    }

    void setStatus(KStatus status) {
        status_ = status;
    }

    bool isRunning() {
        return status_ == kRunning;
    }

    bool isPause() {
        return status_ == kPause;
    }

    bool isStopped() {
        return status_ == kStopped;
    }

private:
    std::atomic<KStatus> status_;
};

} // namespace evpp

} // namespace aru
