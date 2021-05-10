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
 * @file Event.hpp
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
#include <memory>
#include "ars/sdk/event/loop.hpp"

namespace ars {
    
namespace evpp {

struct Event;
struct Timer;

typedef uint64_t            TimerID;
#define INVALID_TIMER_ID    ((TimerID)-1)

typedef std::function<void(Event*)>     EventCallback;
typedef std::function<void(TimerID)>    TimerCallback;

struct Event {
    sdk::event::event_t        event;
    EventCallback   cb;

    Event(EventCallback cb = NULL) {
        memset(&event, 0, sizeof(sdk::event::event_t));
        this->cb = cb;
    }
};

struct Timer {
    sdk::event::timer_t*       timer;
    TimerCallback   cb;
    int             repeat;

    Timer(sdk::event::timer_t* timer = NULL, TimerCallback cb = NULL, int repeat = INFINITE) {
        this->timer = timer;
        this->cb = cb;
        this->repeat = repeat;
    }
};

typedef std::shared_ptr<Event> EventPtr;
typedef std::shared_ptr<Timer> TimerPtr;

} // namespace evpp

} // namespace ars
