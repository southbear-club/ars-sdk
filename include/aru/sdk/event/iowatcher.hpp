#pragma once

#include "loop.hpp"

namespace aru {

namespace sdk {
    
#if !defined(EVENT_SELECT) &&   \
    !defined(EVENT_POLL) &&     \
    !defined(EVENT_EPOLL) &&    \
    !defined(EVENT_KQUEUE) &&   \
    !defined(EVENT_IOCP) &&     \
    !defined(EVENT_PORT) &&     \
    !defined(EVENT_NOEVENT)
#ifdef OS_WIN
// #define EVENT_IOCP // IOCP improving
#define EVENT_POLL
#elif defined(ARU_OS_LINUX)
#define EVENT_EPOLL
#elif defined(ARU_OS_MAC)
#define EVENT_KQUEUE
#elif defined(ARU_OS_BSD)
#define EVENT_KQUEUE
#elif defined(ARU_OS_SOLARIS)
#define EVENT_PORT
#else
#define EVENT_SELECT
#endif
#endif

int iowatcher_init(hloop_t* loop);
int iowatcher_cleanup(hloop_t* loop);
int iowatcher_add_event(hloop_t* loop, int fd, int events);
int iowatcher_del_event(hloop_t* loop, int fd, int events);
int iowatcher_poll_events(hloop_t* loop, int timeout);

} // end sdk
} // end aru