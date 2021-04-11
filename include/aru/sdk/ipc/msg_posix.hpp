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
 * @file msg_posix.hpp
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
#include <time.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>

namespace aru {
    
namespace sdk {

namespace ipc {
    
typedef mqd_t mq_t;

// struct mq_attr {
//     long mq_flags;   /* Flags (ignored for mq_open()) */
//     long mq_maxmsg;  /* Max. # of messages on queue */
//     long mq_msgsize; /* Max. message size (bytes) */
//     long mq_curmsgs; /* # of messages currently in queue
//                         (ignored for mq_open()) */
// };

static inline mq_t mq_open(const char *name, int oflag, mode_t mode, struct mq_attr *attr) {
    return ::mq_open(name, oflag, mode, attr);
}

static inline int mq_close(mq_t h) {
    return ::mq_close(h);
}

static inline int mq_unlink(const char *name) {
    return ::mq_unlink(name);
}

static inline int mq_send(mq_t h, const char *msg_ptr, size_t len, unsigned int prio) {
    return ::mq_send(h, msg_ptr, len, prio);
}

static inline int mq_send_wait(mq_t h, const char *msg_ptr, size_t len, unsigned int prio, const struct timespec *abs_tm) {
    return ::mq_timedsend(h, msg_ptr, len, prio, abs_tm);
}

static inline ssize_t mq_recv(mq_t h, char *msg_ptr, size_t len, unsigned int *prio) {
    return ::mq_receive(h, msg_ptr, len, prio);
}

static inline ssize_t mq_recv_wait(mq_t h, char *msg_ptr, size_t len, unsigned int *prio, const struct timespec *abs_tm) {
    return ::mq_timedreceive(h, msg_ptr, len, prio, abs_tm);
}

static inline int mq_getattr(mq_t h, struct mq_attr *attr) {
    return ::mq_getattr(h, attr);
}

static inline int mq_setattr(mq_t h, const struct mq_attr *newattr, struct mq_attr *oldattr) {
    return ::mq_setattr(h, newattr, oldattr);
}

static inline int mq_notify(mq_t h, const struct sigevent *sevp) {
    return ::mq_notify(h, sevp);
}

} // namespace ipc

} // namespace sdk

} // namespace aru
