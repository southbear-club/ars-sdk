#include "ars/sdk/event/loop.hpp"
#include "ars/sdk/event/event.hpp"
#include "ars/sdk/event/iowatcher.hpp"
#include "ars/sdk/macros/attr.hpp"
#include "ars/sdk/math/math.hpp"
#include "ars/sdk/net/sock.hpp"
#include "ars/sdk/thread/thread.hpp"
#include "ars/sdk/time/time.hpp"

namespace ars {

namespace sdk {

namespace event {

#define HLOOP_PAUSE_TIME 10        // ms
#define HLOOP_MAX_BLOCK_TIME 1000  // ms
#define HLOOP_STAT_TIMEOUT 60000   // ms

#define IO_ARRAY_INIT_SIZE 1024
#define CUSTOM_EVENT_QUEUE_INIT_SIZE 16

#define SOCKPAIR_WRITE_INDEX 0
#define SOCKPAIR_READ_INDEX 1

static void __hidle_del(idle_t* idle);
static void __htimer_del(timer_t* timer);

static int timers_compare(const struct heap_node* lhs, const struct heap_node* rhs) {
    return ARS_TIMER_ENTRY(lhs)->next_timeout < ARS_TIMER_ENTRY(rhs)->next_timeout;
}

static int hloop_process_idles(loop_t* loop) {
    int nidles = 0;
    struct list_node* node = loop->idles.next;
    idle_t* idle = NULL;
    while (node != &loop->idles) {
        idle = ARS_IDLE_ENTRY(node);
        node = node->next;
        if (idle->repeat != INFINITE) {
            --idle->repeat;
        }
        if (idle->repeat == 0) {
            // NOTE: Just mark it as destroy and remove from list.
            // Real deletion occurs after hloop_process_pendings.
            __hidle_del(idle);
        }
        ARS_EVENT_PENDING(idle);
        ++nidles;
    }
    return nidles;
}

static int hloop_process_timers(loop_t* loop) {
    int ntimers = 0;
    timer_t* timer = NULL;
    uint64_t now_hrtime = loop_now_hrtime(loop);
    while (loop->timers.root) {
        // NOTE: root of minheap has min timeout.
        timer = ARS_TIMER_ENTRY(loop->timers.root);
        if (timer->next_timeout > now_hrtime) {
            break;
        }
        if (timer->repeat != INFINITE) {
            --timer->repeat;
        }
        if (timer->repeat == 0) {
            // NOTE: Just mark it as destroy and remove from heap.
            // Real deletion occurs after hloop_process_pendings.
            __htimer_del(timer);
        } else {
            // NOTE: calc next timeout, then re-insert heap.
            heap_dequeue(&loop->timers);
            if (timer->event_type == EVENT_TYPE_TIMEOUT) {
                while (timer->next_timeout <= now_hrtime) {
                    timer->next_timeout += ((timeout_t*)timer)->timeout * 1000;
                }
            } else if (timer->event_type == EVENT_TYPE_PERIOD) {
                period_t* period = (period_t*)timer;
                timer->next_timeout = cron_next_timeout(period->minute, period->hour, period->day,
                                                        period->week, period->month) *
                                      1000000;
            }
            heap_insert(&loop->timers, &timer->node);
        }
        ARS_EVENT_PENDING(timer);
        ++ntimers;
    }
    return ntimers;
}

static int hloop_process_ios(loop_t* loop, int timeout) {
    // That is to call IO multiplexing function such as select, poll, epoll, etc.
    int nevents = iowatcher_poll_events(loop, timeout);
    if (nevents < 0) {
        // hloge("poll_events error=%d", -nevents);
    }
    return nevents < 0 ? 0 : nevents;
}

static int hloop_process_pendings(loop_t* loop) {
    if (loop->npendings == 0) return 0;

    event_t* cur = NULL;
    event_t* next = NULL;
    int ncbs = 0;
    // NOTE: invoke event callback from high to low sorted by priority.
    for (int i = ARS_EVENT_PRIORITY_SIZE - 1; i >= 0; --i) {
        cur = loop->pendings[i];
        while (cur) {
            next = cur->pending_next;
            if (cur->pending) {
                if (cur->active && cur->cb) {
                    cur->cb(cur);
                    ++ncbs;
                }
                cur->pending = 0;
                // NOTE: Now we can safely delete event marked as destroy.
                if (cur->destroy) {
                    ARS_EVENT_DEL(cur);
                }
            }
            cur = next;
        }
        loop->pendings[i] = NULL;
    }
    loop->npendings = 0;
    return ncbs;
}

// hloop_process_ios -> hloop_process_timers -> hloop_process_idles -> hloop_process_pendings
static int hloop_process_events(loop_t* loop) {
    // ios -> timers -> idles
    int ARS_UNUSED(nios);
    int ntimers;
    int nidles;
    nios = ntimers = nidles = 0;

    // calc blocktime
    int32_t blocktime = HLOOP_MAX_BLOCK_TIME;
    if (loop->timers.root) {
        loop_update_time(loop);
        uint64_t next_min_timeout = ARS_TIMER_ENTRY(loop->timers.root)->next_timeout;
        int64_t blocktime_us = next_min_timeout - loop_now_hrtime(loop);
        if (blocktime_us <= 0) goto process_timers;
        blocktime = blocktime_us / 1000;
        ++blocktime;
        blocktime = ARS_MIN(blocktime, HLOOP_MAX_BLOCK_TIME);
    }

    if (loop->nios) {
        nios = hloop_process_ios(loop, blocktime);
    } else {
        msdelay(blocktime);
    }
    loop_update_time(loop);
    // wakeup by loop_stop
    if (loop->status == LOOP_STATUS_STOP) {
        return 0;
    }

process_timers:
    if (loop->ntimers) {
        ntimers = hloop_process_timers(loop);
    }

    int npendings = loop->npendings;
    if (npendings == 0) {
        if (loop->nidles) {
            nidles = hloop_process_idles(loop);
        }
    }
    int ncbs = hloop_process_pendings(loop);
    // printd("blocktime=%d nios=%d/%u ntimers=%d/%u nidles=%d/%u nactives=%d npendings=%d
    // ncbs=%d\n",
    //         blocktime, nios, loop->nios, ntimers, loop->ntimers, nidles, loop->nidles,
    //         loop->nactives, npendings, ncbs);
    return ncbs;
}

static void ARS_UNUSED(hloop_stat_timer_cb)(timer_t* timer) {
    loop_t* ARS_UNUSED(loop) = timer->loop;
    // hlog_set_level(LOG_LEVEL_DEBUG);
    // hlogd("[loop] pid=%ld tid=%ld uptime=%lluus cnt=%llu nactives=%u nios=%d ntimers=%d
    // nidles=%u",
    //     loop->pid, loop->tid, loop->cur_hrtime - loop->start_hrtime, loop->loop_cnt,
    //     loop->nactives, loop->nios, loop->ntimers, loop->nidles);
}

static void sockpair_read_cb(io_t* io, void* buf, int readbytes) {
    loop_t* loop = io->loop;
    event_t* pev = NULL;
    event_t ev;
    for (int i = 0; i < readbytes; ++i) {
        mutex_lock(&loop->custom_events_mutex);
        if (event_queue_empty(&loop->custom_events)) {
            goto unlock;
        }
        pev = event_queue_front(&loop->custom_events);
        if (pev == NULL) {
            goto unlock;
        }
        ev = *pev;
        event_queue_pop_front(&loop->custom_events);
        // NOTE: unlock before cb, avoid deadlock if loop_post_event called in cb.
        mutex_unlock(&loop->custom_events_mutex);
        if (ev.cb) {
            ev.cb(&ev);
        }
    }
    return;
unlock:
    mutex_unlock(&loop->custom_events_mutex);
}

void loop_post_event(loop_t* loop, event_t* ev) {
    char buf = '1';

    if (loop->sockpair[0] == -1 || loop->sockpair[1] == -1) {
        // hlogw("socketpair not created!");
        return;
    }

    if (ev->loop == NULL) {
        ev->loop = loop;
    }
    if (ev->event_type == 0) {
        ev->event_type = EVENT_TYPE_CUSTOM;
    }
    if (ev->event_id == 0) {
        ev->event_id = loop_next_event_id();
    }

    mutex_lock(&loop->custom_events_mutex);
    ev_write(loop, loop->sockpair[SOCKPAIR_WRITE_INDEX], &buf, 1, NULL);
    event_queue_push_back(&loop->custom_events, ev);
    mutex_unlock(&loop->custom_events_mutex);
}

static void hloop_init(loop_t* loop) {
#ifdef OS_WIN
    static int s_wsa_initialized = 0;
    if (s_wsa_initialized == 0) {
        s_wsa_initialized = 1;
        WSADATA wsadata;
        WSAStartup(MAKEWORD(2, 2), &wsadata);
    }
#endif
#ifdef SIGPIPE
    // NOTE: if not ignore SIGPIPE, write twice when peer close will lead to exit process by
    // SIGPIPE.
    signal(SIGPIPE, SIG_IGN);
#endif

    loop->status = LOOP_STATUS_STOP;
    loop->pid = getpid();
    loop->tid = (long)gettid();

    // idles
    list_init(&loop->idles);

    // timers
    heap_init(&loop->timers, timers_compare);

    // ios
    io_array_init(&loop->ios, IO_ARRAY_INIT_SIZE);

    // readbuf
    loop->readbuf.len = ARS_LOOP_READ_BUFSIZE;
    ARS_ALLOC(loop->readbuf.base, loop->readbuf.len);

    // iowatcher
    iowatcher_init(loop);

    // custom_events
    mutex_lock_init(&loop->custom_events_mutex);
    event_queue_init(&loop->custom_events, CUSTOM_EVENT_QUEUE_INIT_SIZE);
    loop->sockpair[0] = loop->sockpair[1] = -1;
    if (socketpair(AF_INET, SOCK_STREAM, 0, loop->sockpair) != 0) {
        // hloge("socketpair create failed!");
    }

    // NOTE: init start_time here, because timer_add use it.
    loop->start_ms = gettimeofday_ms();
    loop->start_hrtime = loop->cur_hrtime = gethrtime_us();
}

static void hloop_cleanup(loop_t* loop) {
    // pendings
    printd("cleanup pendings...\n");
    for (int i = 0; i < ARS_EVENT_PRIORITY_SIZE; ++i) {
        loop->pendings[i] = NULL;
    }

    // ios
    printd("cleanup ios...\n");
    for (size_t i = 0; i < loop->ios.maxsize; ++i) {
        io_t* io = loop->ios.ptr[i];
        if (io) {
            io_free(io);
        }
    }
    io_array_cleanup(&loop->ios);

    // idles
    printd("cleanup idles...\n");
    struct list_node* node = loop->idles.next;
    idle_t* idle;
    while (node != &loop->idles) {
        idle = ARS_IDLE_ENTRY(node);
        node = node->next;
        ARS_FREE(idle);
    }
    list_init(&loop->idles);

    // timers
    printd("cleanup timers...\n");
    timer_t* timer;
    while (loop->timers.root) {
        timer = ARS_TIMER_ENTRY(loop->timers.root);
        heap_dequeue(&loop->timers);
        ARS_FREE(timer);
    }
    heap_init(&loop->timers, NULL);

    // readbuf
    if (loop->readbuf.base && loop->readbuf.len) {
        ARS_FREE(loop->readbuf.base);
        loop->readbuf.base = NULL;
        loop->readbuf.len = 0;
    }

    // iowatcher
    iowatcher_cleanup(loop);

    // custom_events
    mutex_lock(&loop->custom_events_mutex);
    if (loop->sockpair[0] != -1 && loop->sockpair[1] != -1) {
        close(loop->sockpair[0]);
        close(loop->sockpair[1]);
        loop->sockpair[0] = loop->sockpair[1] = -1;
    }
    event_queue_cleanup(&loop->custom_events);
    mutex_unlock(&loop->custom_events_mutex);
    mutex_lock_deinit(&loop->custom_events_mutex);
}

loop_t* loop_new(int flags) {
    loop_t* loop;
    ARS_ALLOC_SIZEOF(loop);
    hloop_init(loop);
    loop->flags |= flags;
    return loop;
}

void loop_free(loop_t** pp) {
    if (pp && *pp) {
        hloop_cleanup(*pp);
        ARS_FREE(*pp);
        *pp = NULL;
    }
}

// while(loop->status) { hloop_process_events(loop); }
int loop_run(loop_t* loop) {
    loop->pid = getpid();
    loop->tid = (long)gettid();

    // intern events
    uint32_t intern_events = 0;
    if (loop->sockpair[0] != -1 && loop->sockpair[1] != -1) {
        ev_read(loop, loop->sockpair[SOCKPAIR_READ_INDEX], loop->readbuf.base, loop->readbuf.len,
                sockpair_read_cb);
        ++intern_events;
    }
#ifdef DEBUG
    timer_add(loop, hloop_stat_timer_cb, HLOOP_STAT_TIMEOUT, INFINITE);
    ++intern_events;
#endif

    loop->status = LOOP_STATUS_RUNNING;
    while (loop->status != LOOP_STATUS_STOP) {
        if (loop->status == LOOP_STATUS_PAUSE) {
            msdelay(HLOOP_PAUSE_TIME);
            loop_update_time(loop);
            continue;
        }
        ++loop->loop_cnt;
        if (loop->nactives <= intern_events &&
            loop->flags & ARS_LOOP_FLAG_QUIT_WHEN_NO_ACTIVE_EVENTS) {
            break;
        }
        hloop_process_events(loop);
        if (loop->flags & ARS_LOOP_FLAG_RUN_ONCE) {
            break;
        }
    }
    loop->status = LOOP_STATUS_STOP;
    loop->end_hrtime = gethrtime_us();

    if (loop->flags & ARS_LOOP_FLAG_AUTO_FREE) {
        hloop_cleanup(loop);
        ARS_FREE(loop);
    }
    return 0;
}

int loop_wakeup(loop_t* loop) {
    event_t ev;
    memset(&ev, 0, sizeof(ev));
    loop_post_event(loop, &ev);
    return 0;
}

static void hloop_stop_event_cb(event_t* ev) { ev->loop->status = LOOP_STATUS_STOP; }

int loop_stop(loop_t* loop) {
    loop->status = LOOP_STATUS_STOP;
    if ((long)gettid() != loop->tid) {
        event_t ev;
        memset(&ev, 0, sizeof(ev));
        ev.priority = ARS_EVENT_HIGHEST_PRIORITY;
        ev.cb = hloop_stop_event_cb;
        loop_post_event(loop, &ev);
    }
    return 0;
}

int loop_pause(loop_t* loop) {
    if (loop->status == LOOP_STATUS_RUNNING) {
        loop->status = LOOP_STATUS_PAUSE;
    }
    return 0;
}

int loop_resume(loop_t* loop) {
    if (loop->status == LOOP_STATUS_PAUSE) {
        loop->status = LOOP_STATUS_RUNNING;
    }
    return 0;
}

loop_status_e loop_status(loop_t* loop) { return loop->status; }

void loop_update_time(loop_t* loop) {
    loop->cur_hrtime = gethrtime_us();
    if (ARS_ABS((int64_t)loop_now(loop) - (int64_t)time(NULL)) > 1) {
        // systemtime changed, we adjust start_ms
        loop->start_ms = gettimeofday_ms() - (loop->cur_hrtime - loop->start_hrtime) / 1000;
    }
}

uint64_t loop_now(loop_t* loop) {
    return loop->start_ms / 1000 + (loop->cur_hrtime - loop->start_hrtime) / 1000000;
}

uint64_t loop_now_ms(loop_t* loop) {
    return loop->start_ms + (loop->cur_hrtime - loop->start_hrtime) / 1000;
}

uint64_t loop_now_hrtime(loop_t* loop) {
    return loop->start_ms * 1000 + (loop->cur_hrtime - loop->start_hrtime);
}

long loop_pid(loop_t* loop) { return loop->pid; }

long loop_tid(loop_t* loop) { return loop->tid; }

void loop_set_userdata(loop_t* loop, void* userdata) { loop->userdata = userdata; }

void* loop_userdata(loop_t* loop) { return loop->userdata; }

idle_t* idle_add(loop_t* loop, idle_cb cb, uint32_t repeat) {
    idle_t* idle;
    ARS_ALLOC_SIZEOF(idle);
    idle->event_type = EVENT_TYPE_IDLE;
    idle->priority = ARS_EVENT_LOWEST_PRIORITY;
    idle->repeat = repeat;
    list_add(&idle->node, &loop->idles);
    ARS_EVENT_ADD(loop, idle, cb);
    loop->nidles++;
    return idle;
}

static void __hidle_del(idle_t* idle) {
    if (idle->destroy) return;
    idle->destroy = 1;
    list_del(&idle->node);
    idle->loop->nidles--;
}

void idle_del(idle_t* idle) {
    if (!idle->active) return;
    __hidle_del(idle);
    ARS_EVENT_DEL(idle);
}

timer_t* timer_add(loop_t* loop, timer_cb cb, uint32_t timeout, uint32_t repeat) {
    if (timeout == 0) return NULL;
    timeout_t* timer;
    ARS_ALLOC_SIZEOF(timer);
    timer->event_type = EVENT_TYPE_TIMEOUT;
    timer->priority = ARS_EVENT_HIGHEST_PRIORITY;
    timer->repeat = repeat;
    timer->timeout = timeout;
    loop_update_time(loop);
    timer->next_timeout = loop_now_hrtime(loop) + timeout * 1000;
    heap_insert(&loop->timers, &timer->node);
    ARS_EVENT_ADD(loop, timer, cb);
    loop->ntimers++;
    return (timer_t*)timer;
}

void timer_reset(timer_t* timer) {
    if (timer->event_type != EVENT_TYPE_TIMEOUT) {
        return;
    }
    loop_t* loop = timer->loop;
    timeout_t* timeout = (timeout_t*)timer;
    if (timer->destroy) {
        loop->ntimers++;
    } else {
        heap_remove(&loop->timers, &timer->node);
    }
    if (timer->repeat == 0) {
        timer->repeat = 1;
    }
    timer->next_timeout = loop_now_hrtime(loop) + timeout->timeout * 1000;
    heap_insert(&loop->timers, &timer->node);
    ARS_EVENT_RESET(timer);
}

timer_t* timer_add_period(loop_t* loop, timer_cb cb, int8_t minute, int8_t hour, int8_t day,
                          int8_t week, int8_t month, uint32_t repeat) {
    if (minute > 59 || hour > 23 || day > 31 || week > 6 || month > 12) {
        return NULL;
    }
    period_t* timer;
    ARS_ALLOC_SIZEOF(timer);
    timer->event_type = EVENT_TYPE_PERIOD;
    timer->priority = ARS_EVENT_HIGH_PRIORITY;
    timer->repeat = repeat;
    timer->minute = minute;
    timer->hour = hour;
    timer->day = day;
    timer->month = month;
    timer->week = week;
    timer->next_timeout = cron_next_timeout(minute, hour, day, week, month) * 1000000;
    heap_insert(&loop->timers, &timer->node);
    ARS_EVENT_ADD(loop, timer, cb);
    loop->ntimers++;
    return (timer_t*)timer;
}

static void __htimer_del(timer_t* timer) {
    if (timer->destroy) return;
    heap_remove(&timer->loop->timers, &timer->node);
    timer->loop->ntimers--;
    timer->destroy = 1;
}

void timer_del(timer_t* timer) {
    if (!timer->active) return;
    __htimer_del(timer);
    ARS_EVENT_DEL(timer);
}

const char* io_engine() {
#ifdef EVENT_SELECT
    return "select";
#elif defined(EVENT_POLL)
    return "poll";
#elif defined(EVENT_EPOLL)
    return "epoll";
#elif defined(EVENT_KQUEUE)
    return "kqueue";
#elif defined(EVENT_IOCP)
    return "iocp";
#elif defined(EVENT_PORT)
    return "evport";
#else
    return "noevent";
#endif
}

static void fill_io_type(io_t* io) {
    int type = 0;
    socklen_t optlen = sizeof(int);
    int ret = getsockopt(io->fd, SOL_SOCKET, SO_TYPE, (char*)&type, &optlen);
    printd("getsockopt SO_TYPE fd=%d ret=%d type=%d errno=%d\n", io->fd, ret, type, errno);
    if (ret == 0) {
        switch (type) {
            case SOCK_STREAM:
                io->io_type = IO_TYPE_TCP;
                break;
            case SOCK_DGRAM:
                io->io_type = IO_TYPE_UDP;
                break;
            case SOCK_RAW:
                io->io_type = IO_TYPE_IP;
                break;
            default:
                io->io_type = IO_TYPE_SOCKET;
                break;
        }
    } else if (errno == ENOTSOCK) {
        switch (io->fd) {
            case 0:
                io->io_type = IO_TYPE_STDIN;
                break;
            case 1:
                io->io_type = IO_TYPE_STDOUT;
                break;
            case 2:
                io->io_type = IO_TYPE_STDERR;
                break;
            default:
                io->io_type = IO_TYPE_FILE;
                break;
        }
    } else {
        io->io_type = IO_TYPE_TCP;
    }
}

static void hio_socket_init(io_t* io) {
    // nonblocking
    sock_set_nonblock(io->fd);
    // fill io->localaddr io->peeraddr
    if (io->localaddr == NULL) {
        ARS_ALLOC(io->localaddr, sizeof(sock_addr_t));
    }
    if (io->peeraddr == NULL) {
        ARS_ALLOC(io->peeraddr, sizeof(sock_addr_t));
    }
    socklen_t addrlen = sizeof(sock_addr_t);
    int ARS_UNUSED(ret) = getsockname(io->fd, io->localaddr, &addrlen);
    printd("getsockname fd=%d ret=%d errno=%d\n", io->fd, ret, errno);
    // NOTE:
    // tcp_server peeraddr set by accept
    // udp_server peeraddr set by recvfrom
    // tcp_client/udp_client peeraddr set by hio_setpeeraddr
    if (io->io_type == IO_TYPE_TCP || io->io_type == IO_TYPE_SSL) {
        // tcp acceptfd
        addrlen = sizeof(sock_addr_t);
        ret = getpeername(io->fd, io->peeraddr, &addrlen);
        printd("getpeername fd=%d ret=%d errno=%d\n", io->fd, ret, socket_errno());
    }
}

void io_init(io_t* io) {
    // alloc localaddr,peeraddr when hio_socket_init
    /*
    if (io->localaddr == NULL) {
        HV_ALLOC(io->localaddr, sizeof(sockaddr_u));
    }
    if (io->peeraddr == NULL) {
        HV_ALLOC(io->peeraddr, sizeof(sockaddr_u));
    }
    */

    // write_queue init when ev_write try_write failed
    // write_queue_init(&io->write_queue, 4);

    mutex_lock_init(&io->write_mutex);
}

void io_ready(io_t* io) {
    if (io->ready) return;
    // flags
    io->ready = 1;
    io->closed = 0;
    io->accept = io->connect = io->connectex = 0;
    io->recv = io->send = 0;
    io->recvfrom = io->sendto = 0;
    io->close = 0;
    // public:
    io->id = io_next_id();
    io->io_type = IO_TYPE_UNKNOWN;
    io->error = 0;
    io->events = io->revents = 0;
    // callbacks
    io->read_cb = NULL;
    io->write_cb = NULL;
    io->close_cb = NULL;
    io->accept_cb = NULL;
    io->connect_cb = NULL;
    // timers
    io->connect_timeout = 0;
    io->connect_timer = NULL;
    io->close_timeout = 0;
    io->close_timer = NULL;
    io->keepalive_timeout = 0;
    io->keepalive_timer = NULL;
    io->heartbeat_interval = 0;
    io->heartbeat_fn = NULL;
    io->heartbeat_timer = NULL;
    // upstream
    io->upstream_io = NULL;
    // private:
    io->event_index[0] = io->event_index[1] = -1;
    io->hovlp = NULL;
    io->ssl = NULL;

    // io_type
    fill_io_type(io);
    if (io->io_type & IO_TYPE_SOCKET) {
        hio_socket_init(io);
    }
}

void io_done(io_t* io) {
    if (!io->ready) return;
    io->ready = 0;

    hio_del(io, ARS_IO_RDWR);

    offset_buf_t* pbuf = NULL;
    mutex_lock(&io->write_mutex);
    while (!write_queue_empty(&io->write_queue)) {
        pbuf = write_queue_front(&io->write_queue);
        ARS_FREE(pbuf->base);
        write_queue_pop_front(&io->write_queue);
    }
    write_queue_cleanup(&io->write_queue);
    mutex_unlock(&io->write_mutex);
}

void io_free(io_t* io) {
    if (io == NULL) return;
    // NOTE: call io_done to cleanup write_queue
    io_done(io);
    // NOTE: call io_close to call close_cb
    io_close(io);
    mutex_lock_deinit(&io->write_mutex);
    ARS_FREE(io->localaddr);
    ARS_FREE(io->peeraddr);
    ARS_FREE(io);
}

bool io_is_opened(io_t* io) {
    if (io == NULL) return false;
    return io->ready == 1 && io->closed == 0;
}

bool io_is_closed(io_t* io) {
    if (io == NULL) return true;
    return io->ready == 0 && io->closed == 1;
}

io_t* io_get(loop_t* loop, int fd) {
    if (fd < 0) return nullptr;
    if ((uint32_t)fd >= loop->ios.maxsize) {
        int newsize = ceil2e(fd);
        io_array_resize(&loop->ios, newsize > fd ? newsize : 2 * fd);
    }

    io_t* io = loop->ios.ptr[fd];
    if (io == NULL) {
        ARS_ALLOC_SIZEOF(io);
        io_init(io);
        io->event_type = EVENT_TYPE_IO;
        io->loop = loop;
        io->fd = fd;
        loop->ios.ptr[fd] = io;
    }

    if (!io->ready) {
        io_ready(io);
    }

    return io;
}

int io_add(io_t* io, io_cb cb, int events) {
    printd("io_add fd=%d io->events=%d events=%d\n", io->fd, io->events, events);
#ifdef OS_WIN
    // Windows iowatcher not work on stdio
    if (io->fd < 3) return -1;
#endif
    loop_t* loop = io->loop;
    if (!io->active) {
        ARS_EVENT_ADD(loop, io, cb);
        loop->nios++;
    }

    if (!io->ready) {
        io_ready(io);
    }

    if (cb) {
        io->cb = (event_cb)cb;
    }

    if (!(io->events & events)) {
        iowatcher_add_event(loop, io->fd, events);
        io->events |= events;
    }
    return 0;
}

int hio_del(io_t* io, int events) {
    printd("hio_del fd=%d io->events=%d events=%d\n", io->fd, io->events, events);
#ifdef OS_WIN
    // Windows iowatcher not work on stdio
    if (io->fd < 3) return -1;
#endif
    if (!io->active) return -1;

    if (io->events & events) {
        iowatcher_del_event(io->loop, io->fd, events);
        io->events &= ~events;
    }
    if (io->events == 0) {
        io->loop->nios--;
        // NOTE: not ARS_EVENT_DEL, avoid free
        ARS_EVENT_INACTIVE(io);
    }
    return 0;
}

io_t* ev_read(loop_t* loop, int fd, void* buf, size_t len, read_cb read_cb) {
    io_t* io = io_get(loop, fd);
    assert(io != NULL);
    io->readbuf.base = (char*)buf;
    io->readbuf.len = len;
    if (read_cb) {
        io->read_cb = read_cb;
    }
    io_read(io);
    return io;
}

io_t* ev_write(loop_t* loop, int fd, const void* buf, size_t len, write_cb write_cb) {
    io_t* io = io_get(loop, fd);
    assert(io != NULL);
    if (write_cb) {
        io->write_cb = write_cb;
    }
    io_write(io, buf, len);
    return io;
}

io_t* ev_accept(loop_t* loop, int listenfd, accept_cb accept_cb) {
    io_t* io = io_get(loop, listenfd);
    assert(io != NULL);
    if (accept_cb) {
        io->accept_cb = accept_cb;
    }
    io_accept(io);
    return io;
}

io_t* ev_connect(loop_t* loop, int connfd, connect_cb connect_cb) {
    io_t* io = io_get(loop, connfd);
    assert(io != NULL);
    if (connect_cb) {
        io->connect_cb = connect_cb;
    }
    io_connect(io);
    return io;
}

void ev_close(loop_t* loop, int fd) {
    io_t* io = io_get(loop, fd);
    assert(io != NULL);
    io_close(io);
}

io_t* ev_recv(loop_t* loop, int connfd, void* buf, size_t len, read_cb read_cb) {
    // io_t* io = io_get(loop, connfd);
    // assert(io != NULL);
    // io->recv = 1;
    // if (io->io_type != IO_TYPE_SSL) {
    // io->io_type = IO_TYPE_TCP;
    //}
    return ev_read(loop, connfd, buf, len, read_cb);
}

io_t* ev_send(loop_t* loop, int connfd, const void* buf, size_t len, write_cb write_cb) {
    // io_t* io = io_get(loop, connfd);
    // assert(io != NULL);
    // io->send = 1;
    // if (io->io_type != IO_TYPE_SSL) {
    // io->io_type = IO_TYPE_TCP;
    //}
    return ev_write(loop, connfd, buf, len, write_cb);
}

io_t* ev_recvfrom(loop_t* loop, int sockfd, void* buf, size_t len, read_cb read_cb) {
    // io_t* io = io_get(loop, sockfd);
    // assert(io != NULL);
    // io->recvfrom = 1;
    // io->io_type = IO_TYPE_UDP;
    return ev_read(loop, sockfd, buf, len, read_cb);
}

io_t* ev_sendto(loop_t* loop, int sockfd, const void* buf, size_t len, write_cb write_cb) {
    // io_t* io = io_get(loop, sockfd);
    // assert(io != NULL);
    // io->sendto = 1;
    // io->io_type = IO_TYPE_UDP;
    return ev_write(loop, sockfd, buf, len, write_cb);
}

io_t* ev_create(loop_t* loop, const char* host, int port, int type) {
    sock_addr_t peeraddr;
    memset(&peeraddr, 0, sizeof(peeraddr));
    int ret = sock_set_ipport(&peeraddr, host, port);
    if (ret != 0) {
        // printf("unknown host: %s\n", host);
        return NULL;
    }
    int connfd = socket(peeraddr.sa.sa_family, type, 0);
    if (connfd < 0) {
        perror("socket");
        return NULL;
    }

    io_t* io = io_get(loop, connfd);
    assert(io != NULL);
    io_set_peeraddr(io, &peeraddr.sa, sock_addr_len(&peeraddr));
    return io;
}

io_t* loop_create_tcp_server(loop_t* loop, const char* host, int port, accept_cb accept_cb) {
    int listenfd = sock_fast_listen(port, host);
    if (listenfd < 0) {
        return NULL;
    }
    io_t* io = ev_accept(loop, listenfd, accept_cb);
    if (io == NULL) {
        close(listenfd);
    }
    return io;
}

io_t* loop_create_tcp_client(loop_t* loop, const char* host, int port, connect_cb connect_cb) {
    io_t* io = ev_create(loop, host, port, SOCK_STREAM);
    if (io == NULL) return NULL;
    ev_connect(loop, io->fd, connect_cb);
    return io;
}

io_t* loop_create_udp_server(loop_t* loop, const char* host, int port) {
    int bindfd = sock_fast_bind(port, host, SOCK_DGRAM);
    if (bindfd < 0) {
        return NULL;
    }
    return io_get(loop, bindfd);
}

io_t* loop_create_udp_client(loop_t* loop, const char* host, int port) {
    return ev_create(loop, host, port, SOCK_DGRAM);
}

// upstream
void io_read_upstream(io_t* io) {
    io_t* upstream_io = io->upstream_io;
    if (upstream_io) {
        io_read(io);
        io_read(upstream_io);
    }
}

void io_write_upstream(io_t* io, void* buf, int bytes) {
    io_t* upstream_io = io->upstream_io;
    if (upstream_io) {
        io_write(upstream_io, buf, bytes);
    }
}

void io_close_upstream(io_t* io) {
    io_t* upstream_io = io->upstream_io;
    if (upstream_io) {
        io_close(upstream_io);
    }
}

void io_setup_upstream(io_t* io1, io_t* io2) {
    io1->upstream_io = io2;
    io2->upstream_io = io1;
    io_setcb_read(io1, io_write_upstream);
    io_setcb_read(io2, io_write_upstream);
}

io_t* io_get_upstream(io_t* io) { return io->upstream_io; }

io_t* io_setup_tcp_upstream(io_t* io, const char* host, int port, int ssl) {
    io_t* upstream_io = ev_create(io->loop, host, port, SOCK_STREAM);
    if (upstream_io == NULL) return NULL;
    if (ssl) io_enable_ssl(upstream_io);
    io_setup_upstream(io, upstream_io);
    io_setcb_close(io, io_close_upstream);
    io_setcb_close(upstream_io, io_close_upstream);
    ev_connect(io->loop, upstream_io->fd, io_read_upstream);
    return upstream_io;
}

io_t* io_setup_udp_upstream(io_t* io, const char* host, int port) {
    io_t* upstream_io = ev_create(io->loop, host, port, SOCK_DGRAM);
    if (upstream_io == NULL) return NULL;
    io_setup_upstream(io, upstream_io);
    io_read_upstream(io);
    return upstream_io;
}

}  // namespace event

}  // namespace sdk

}  // namespace ars
