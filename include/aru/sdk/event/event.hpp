#pragma once

#include "../ds/array.hpp"
#include "../ds/buf.hpp"
#include "../ds/heap.hpp"
#include "../ds/list.hpp"
#include "../ds/queue.hpp"
#include "../lock/lock.hpp"
#include "../memory/mem.hpp"
#include "loop.hpp"

namespace aru {

namespace sdk {

namespace event {

#define ARU_LOOP_READ_BUFSIZE 8192

ARU_ARRAY_DECL(io_t*, io_array);
ARU_QUEUE_DECL(event_t, event_queue);

struct loop_s {
    uint32_t flags;
    loop_status_e status;
    uint64_t start_ms;      // ms
    uint64_t start_hrtime;  // us
    uint64_t end_hrtime;
    uint64_t cur_hrtime;
    uint64_t loop_cnt;
    long pid;
    long tid;
    void* userdata;
    // private:
    // events
    uint32_t nactives;
    uint32_t npendings;
    // pendings: with priority as array.index
    event_t* pendings[ARU_EVENT_PRIORITY_SIZE];
    // idles
    struct list_head idles;
    uint32_t nidles;
    // timers
    struct heap timers;
    uint32_t ntimers;
    // ios: with fd as array.index
    struct io_array ios;
    uint32_t nios;
    // one loop per thread, so one readbuf per loop is OK.
    buf_t readbuf;
    void* iowatcher;
    // custom_events
    int sockpair[2];
    event_queue custom_events;
    mutex_lock_t custom_events_mutex;
};

uint64_t loop_next_event_id();

struct idle_s {
    ARU_EVENT_FIELDS
    uint32_t repeat;
    // private:
    struct list_node node;
};

#define ARU_TIMER_FIELDS   \
    ARU_EVENT_FIELDS       \
    uint32_t repeat;       \
    uint64_t next_timeout; \
    struct heap_node node;

struct timer_s {
    ARU_TIMER_FIELDS
};

struct timeout_s {
    ARU_TIMER_FIELDS
    uint32_t timeout;
};

struct period_s {
    ARU_TIMER_FIELDS
    int8_t minute;
    int8_t hour;
    int8_t day;
    int8_t week;
    int8_t month;
};

ARU_QUEUE_DECL(offset_buf_t, write_queue);

struct io_s {
    ARU_EVENT_FIELDS
    // flags
    unsigned ready : 1;
    unsigned closed : 1;
    unsigned accept : 1;
    unsigned connect : 1;
    unsigned connectex : 1;  // for ConnectEx/DisconnectEx
    unsigned recv : 1;
    unsigned send : 1;
    unsigned recvfrom : 1;
    unsigned sendto : 1;
    unsigned close : 1;
    // public:
    uint32_t id;  // fd cannot be used as unique identifier, so we provide an id
    int fd;
    io_type_e io_type;
    int error;
    int events;
    int revents;
    struct sockaddr* localaddr;
    struct sockaddr* peeraddr;
    buf_t readbuf;                   // for ev_read
    struct write_queue write_queue;  // for ev_write
    mutex_lock_t write_mutex;        // lock write and write_queue
    // callbacks
    event::read_cb read_cb;
    event::write_cb write_cb;
    event::close_cb close_cb;
    event::accept_cb accept_cb;
    event::connect_cb connect_cb;
    // timers
    int connect_timeout;  // ms
    timer_t* connect_timer;
    int close_timeout;  // ms
    timer_t* close_timer;
    int keepalive_timeout;  // ms
    timer_t* keepalive_timer;
    int heartbeat_interval;  // ms
    hio_send_heartbeat_fn heartbeat_fn;
    timer_t* heartbeat_timer;
    // upstream
    struct io_s* upstream_io;
    // private:
    int event_index[2];  // for poll,kqueue
    void* hovlp;         // for iocp/overlapio
    void* ssl;           // for SSL
    void* ctx;
};
/*
 * hio lifeline:
 * fd =>
 * io_get => HV_ALLOC_SIZEOF(io) => io_init =>
 * io_ready => io_add => hio_del => io_done =>
 * io_close => close_cb =>
 * io_free => HV_FREE(io)
 */
void io_init(io_t* io);
void io_ready(io_t* io);
void io_done(io_t* io);
void io_free(io_t* io);
uint32_t io_next_id();

#define ARU_EVENT_ENTRY(p) container_of(p, event_t, pending_node)
#define ARU_IDLE_ENTRY(p) container_of(p, idle_t, node)
#define ARU_TIMER_ENTRY(p) container_of(p, timer_t, node)

#define ARU_EVENT_ACTIVE(ev)  \
    if (!ev->active) {        \
        ev->active = 1;       \
        ev->loop->nactives++; \
    }

#define ARU_EVENT_INACTIVE(ev) \
    if (ev->active) {          \
        ev->active = 0;        \
        ev->loop->nactives--;  \
    }

#define ARU_EVENT_PENDING(ev)                                                              \
    do {                                                                                   \
        if (!ev->pending) {                                                                \
            ev->pending = 1;                                                               \
            ev->loop->npendings++;                                                         \
            event_t** phead = &ev->loop->pendings[ARU_EVENT_PRIORITY_INDEX(ev->priority)]; \
            ev->pending_next = *phead;                                                     \
            *phead = (event_t*)ev;                                                         \
        }                                                                                  \
    } while (0)

#define ARU_EVENT_ADD(loop, ev, cb)          \
    do {                                     \
        ev->loop = loop;                     \
        ev->event_id = loop_next_event_id(); \
        ev->cb = (event_cb)cb;               \
        ARU_EVENT_ACTIVE(ev);                \
    } while (0)

#define ARU_EVENT_DEL(ev)       \
    do {                        \
        ARU_EVENT_INACTIVE(ev); \
        if (!ev->pending) {     \
            ARU_FREE(ev);       \
        }                       \
    } while (0)

#define ARU_EVENT_RESET(ev)   \
    do {                      \
        ev->destroy = 0;      \
        ARU_EVENT_ACTIVE(ev); \
        ev->pending = 0;      \
    } while (0)

}  // namespace event

}  // namespace sdk

}  // namespace aru
