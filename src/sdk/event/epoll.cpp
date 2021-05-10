#include "ars/sdk/event/iowatcher.hpp"

#ifdef EVENT_EPOLL
#include "ars/sdk/ds/array.hpp"
#include "ars/sdk/event/event.hpp"
#include "ars/sdk/macros/defs.hpp"
#include "ars/sdk/memory/mem.hpp"

#include <sys/epoll.h>
#include <unistd.h>

namespace ars {

namespace sdk {

namespace event {

#define EVENTS_INIT_SIZE 64
ARS_ARRAY_DECL(struct epoll_event, events);

typedef struct epoll_ctx_s {
    int epfd;
    struct events events;
} epoll_ctx_t;

int iowatcher_init(loop_t* loop) {
    if (loop->iowatcher) return 0;
    epoll_ctx_t* epoll_ctx;
    ARS_ALLOC_SIZEOF(epoll_ctx);
    epoll_ctx->epfd = epoll_create(EVENTS_INIT_SIZE);
    events_init(&epoll_ctx->events, EVENTS_INIT_SIZE);
    loop->iowatcher = epoll_ctx;
    return 0;
}

int iowatcher_cleanup(loop_t* loop) {
    if (loop->iowatcher == NULL) return 0;
    epoll_ctx_t* epoll_ctx = (epoll_ctx_t*)loop->iowatcher;
    close(epoll_ctx->epfd);
    events_cleanup(&epoll_ctx->events);
    ARS_FREE(loop->iowatcher);
    return 0;
}

int iowatcher_add_event(loop_t* loop, int fd, int events) {
    if (loop->iowatcher == NULL) {
        iowatcher_init(loop);
    }
    epoll_ctx_t* epoll_ctx = (epoll_ctx_t*)loop->iowatcher;
    io_t* io = loop->ios.ptr[fd];

    struct epoll_event ee;
    memset(&ee, 0, sizeof(ee));
    ee.data.fd = fd;
    // pre events
    if (io->events & ARS_IO_READ) {
        ee.events |= EPOLLIN;
    }
    if (io->events & ARS_IO_WRITE) {
        ee.events |= EPOLLOUT;
    }
    // now events
    if (events & ARS_IO_READ) {
        ee.events |= EPOLLIN;
    }
    if (events & ARS_IO_WRITE) {
        ee.events |= EPOLLOUT;
    }
    int op = io->events == 0 ? EPOLL_CTL_ADD : EPOLL_CTL_MOD;
    epoll_ctl(epoll_ctx->epfd, op, fd, &ee);
    if (op == EPOLL_CTL_ADD) {
        if (epoll_ctx->events.size == epoll_ctx->events.maxsize) {
            events_double_resize(&epoll_ctx->events);
        }
        epoll_ctx->events.size++;
    }
    return 0;
}

int iowatcher_del_event(loop_t* loop, int fd, int events) {
    epoll_ctx_t* epoll_ctx = (epoll_ctx_t*)loop->iowatcher;
    if (epoll_ctx == NULL) return 0;
    io_t* io = loop->ios.ptr[fd];

    struct epoll_event ee;
    memset(&ee, 0, sizeof(ee));
    ee.data.fd = fd;
    // pre events
    if (io->events & ARS_IO_READ) {
        ee.events |= EPOLLIN;
    }
    if (io->events & ARS_IO_WRITE) {
        ee.events |= EPOLLOUT;
    }
    // now events
    if (events & ARS_IO_READ) {
        ee.events &= ~EPOLLIN;
    }
    if (events & ARS_IO_WRITE) {
        ee.events &= ~EPOLLOUT;
    }
    int op = ee.events == 0 ? EPOLL_CTL_DEL : EPOLL_CTL_MOD;
    epoll_ctl(epoll_ctx->epfd, op, fd, &ee);
    if (op == EPOLL_CTL_DEL) {
        epoll_ctx->events.size--;
    }
    return 0;
}

int iowatcher_poll_events(loop_t* loop, int timeout) {
    epoll_ctx_t* epoll_ctx = (epoll_ctx_t*)loop->iowatcher;
    if (epoll_ctx == NULL) return 0;
    if (epoll_ctx->events.size == 0) return 0;
    int nepoll =
        epoll_wait(epoll_ctx->epfd, epoll_ctx->events.ptr, epoll_ctx->events.size, timeout);
    if (nepoll < 0) {
        perror("epoll");
        return nepoll;
    }
    if (nepoll == 0) return 0;
    int nevents = 0;
    for (size_t i = 0; i < epoll_ctx->events.size; ++i) {
        struct epoll_event* ee = epoll_ctx->events.ptr + i;
        int fd = ee->data.fd;
        uint32_t revents = ee->events;
        if (revents) {
            ++nevents;
            io_t* io = loop->ios.ptr[fd];
            if (io) {
                if (revents & (EPOLLIN | EPOLLHUP | EPOLLERR)) {
                    io->revents |= ARS_IO_READ;
                }
                if (revents & (EPOLLOUT | EPOLLHUP | EPOLLERR)) {
                    io->revents |= ARS_IO_WRITE;
                }
                ARS_EVENT_PENDING(io);
            }
        }
        if (nevents == nepoll) break;
    }
    return nevents;
}

}  // namespace event

}  // namespace sdk

}  // namespace ars

#endif
