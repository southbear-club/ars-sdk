#pragma once

#include "loop.hpp"

namespace ars {

namespace sdk {

namespace event {

#if defined(ARS_OS_LINUX)
#define EVENT_EPOLL
#elif defined(ARS_OS_MAC)
#define EVENT_KQUEUE
#else
#error "not support"
#endif

int iowatcher_init(loop_t* loop);
int iowatcher_cleanup(loop_t* loop);
int iowatcher_add_event(loop_t* loop, int fd, int events);
int iowatcher_del_event(loop_t* loop, int fd, int events);
int iowatcher_poll_events(loop_t* loop, int timeout);

}  // namespace event

}  // namespace sdk

}  // namespace ars
