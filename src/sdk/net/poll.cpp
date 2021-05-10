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
 * @file poll.cpp
 * @brief 
 * @author wotsen (astralrovers@outlook.com)
 * @version 1.0.0
 * @date 2021-04-04
 * 
 * @copyright MIT
 * 
 */
#include "ars/sdk/net/poll.hpp"

namespace ars {

namespace sdk {

int poll_wait(int fd, char how, time_t w) {
    int event = poll_err;

    if (how == 'r') {
        event |= poll_in;
    } else if (how == 'w') {
        event |= poll_out;
    } else {
        return -1;
    }

    struct pollfd fds;
    fds.fd = fd;
    fds.events = event;
    fds.revents = 0;

    int r = sdk::poll(&fds, 1, w);

    // 异常
    if (r < 0) {
        return r;
    }

    // 超时
    if (r == 0) {
        return 0;
    }

    // 对端异常或套接字异常
    if ((fds.revents & POLLHUP) || (fds.revents & POLLERR)) {
        return -1;
    }

    // 套接字不匹配
    if (fds.fd != fd) {
        return 0;
    }

    if (how == 'r') {
        if (fds.revents & poll_in) {
            return 1;
        }
    } else if (how == 'w') {
        event |= poll_out;
        if (fds.revents & poll_out) {
            return 1;
        }
    } else {

    }

    return 0;
}

}

}
