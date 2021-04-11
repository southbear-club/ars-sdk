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
 * @file fifo.hpp
 * @brief 
 * @author  ()
 * @version 1.0.0
 * @date 2021-04-10
 * 
 * @copyright MIT
 * 
 */
#pragma once
#include <stddef.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

namespace aru {
    
namespace sdk {

namespace ipc {

typedef int fifo_t;

static inline int mkfifo(const char *path, mode_t mode) {
    return ::mkfifo(path, mode);
}

static inline fifo_t fifo_open(const char *path, int flags) {
    return ::open(path, flags);
}

static inline int fifo_close(fifo_t h) {
    return ::close(h);
}

static inline int fifo_set_nonblock(fifo_t h, bool en = true) {
    int f = fcntl(h, F_GETFL, 0);
    if (f < 0) { return f; }

    if (en) {
        f |= O_NONBLOCK;
    } else {
        f &= ~O_NONBLOCK;
    }

    return fcntl(h, F_SETFL, f);
}

static inline int fifo_write(fifo_t h, const void *data, size_t len) {
    return ::write(h, data, len);
}

static inline ssize_t fifo_read(fifo_t h, void *data, size_t len) {
    return ::read(h, data, len);
}

} // namespace ipc

} // namespace sdk

} // namespace aru
