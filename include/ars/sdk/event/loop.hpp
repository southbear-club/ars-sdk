#pragma once

#include <stdint.h>
#include <sys/socket.h>
#include "../macros/defs.hpp"

namespace ars {

namespace sdk {

namespace event {

typedef struct loop_s loop_t;
typedef struct event_s event_t;

// NOTE: The following structures are subclasses of event_t,
// inheriting event_t data members and function members.
typedef struct idle_s idle_t;
typedef struct timer_s timer_t;
typedef struct timeout_s timeout_t;
typedef struct period_s period_t;
typedef struct io_s io_t;

typedef void (*event_cb)(event_t* ev);
typedef void (*idle_cb)(idle_t* idle);
typedef void (*timer_cb)(timer_t* timer);
typedef void (*io_cb)(io_t* io);

typedef void (*accept_cb)(io_t* io);
typedef void (*connect_cb)(io_t* io);
typedef void (*read_cb)(io_t* io, void* buf, int readbytes);
typedef void (*write_cb)(io_t* io, const void* buf, int writebytes);
typedef void (*close_cb)(io_t* io);

typedef enum { LOOP_STATUS_STOP, LOOP_STATUS_RUNNING, LOOP_STATUS_PAUSE } loop_status_e;

typedef enum {
    EVENT_TYPE_NONE = 0,
    EVENT_TYPE_IO = 0x00000001,
    EVENT_TYPE_TIMEOUT = 0x00000010,
    EVENT_TYPE_PERIOD = 0x00000020,
    EVENT_TYPE_TIMER = EVENT_TYPE_TIMEOUT | EVENT_TYPE_PERIOD,
    EVENT_TYPE_IDLE = 0x00000100,
    EVENT_TYPE_CUSTOM = 0x00000400,  // 1024
} event_type_e;

#define ARS_EVENT_LOWEST_PRIORITY (-5)
#define ARS_EVENT_LOW_PRIORITY (-3)
#define ARS_EVENT_NORMAL_PRIORITY 0
#define ARS_EVENT_HIGH_PRIORITY 3
#define ARS_EVENT_HIGHEST_PRIORITY 5
#define ARS_EVENT_PRIORITY_SIZE (ARS_EVENT_HIGHEST_PRIORITY - ARS_EVENT_LOWEST_PRIORITY + 1)
#define ARS_EVENT_PRIORITY_INDEX(priority) (priority - ARS_EVENT_LOWEST_PRIORITY)

#define ARS_EVENT_FLAGS   \
    unsigned destroy : 1; \
    unsigned active : 1;  \
    unsigned pending : 1;

#define ARS_EVENT_FIELDS          \
    loop_t* loop;                 \
    event_type_e event_type;      \
    uint64_t event_id;            \
    event_cb cb;                  \
    void* userdata;               \
    void* privdata;               \
    int priority;                 \
    struct event_s* pending_next; \
    ARS_EVENT_FLAGS

struct event_s {
    ARS_EVENT_FIELDS
};

#define ars_event_set_priority(ev, prio) ((ars::sdk::event::event_t*)(ev))->priority = prio
#define ars_event_set_userdata(ev, udata) ((ars::sdk::event::event_t*)(ev))->userdata = (void*)udata

#define ars_event_loop(ev) (((ars::sdk::event::event_t*)(ev))->loop)
#define ars_event_type(ev) (((ars::sdk::event::event_t*)(ev))->event_type)
#define ars_event_id(ev) (((ars::sdk::event::event_t*)(ev))->event_id)
#define ars_event_priority(ev) (((ars::sdk::event::event_t*)(ev))->priority)
#define ars_event_userdata(ev) (((ars::sdk::event::event_t*)(ev))->userdata)

typedef enum {
    IO_TYPE_UNKNOWN = 0,
    IO_TYPE_STDIN = 0x00000001,
    IO_TYPE_STDOUT = 0x00000002,
    IO_TYPE_STDERR = 0x00000004,
    IO_TYPE_STDIO = 0x0000000F,

    IO_TYPE_FILE = 0x00000010,

    IO_TYPE_IP = 0x00000100,
    IO_TYPE_UDP = 0x00001000,
    IO_TYPE_TCP = 0x00010000,
    IO_TYPE_SSL = 0x00020000,
    IO_TYPE_SOCKET = 0x00FFFF00,
} io_type_e;

#define ARS_IO_DEFAULT_CONNECT_TIMEOUT 5000      // ms
#define ARS_IO_DEFAULT_CLOSE_TIMEOUT 60000       // ms
#define ARS_IO_DEFAULT_KEEPALIVE_TIMEOUT 75000   // ms
#define ARS_IO_DEFAULT_HEARTBEAT_INTERVAL 30000  // ms

// loop
#define ARS_LOOP_FLAG_RUN_ONCE 0x00000001
#define ARS_LOOP_FLAG_AUTO_FREE 0x00000002
#define ARS_LOOP_FLAG_QUIT_WHEN_NO_ACTIVE_EVENTS 0x00000004
loop_t* loop_new(int flags = ARS_LOOP_FLAG_AUTO_FREE);

// WARN: Forbid to call loop_free if ARS_LOOP_FLAG_AUTO_FREE set.
void loop_free(loop_t** pp);

// NOTE: when no active events, loop will quit if ARS_LOOP_FLAG_QUIT_WHEN_NO_ACTIVE_EVENTS set.
int loop_run(loop_t* loop);
// NOTE: loop_stop called in loop-thread just set flag to quit in next loop,
// if called in other thread, it will wakeup loop-thread from blocking poll system call,
// then you should join loop thread to safely exit loop thread.
int loop_stop(loop_t* loop);
int loop_pause(loop_t* loop);
int loop_resume(loop_t* loop);
int loop_wakeup(loop_t* loop);
loop_status_e loop_status(loop_t* loop);

void loop_update_time(loop_t* loop);
uint64_t loop_now(loop_t* loop);         // s
uint64_t loop_now_ms(loop_t* loop);      // ms
uint64_t loop_now_hrtime(loop_t* loop);  // us
#define ars_loop_now_us ars::sdk::event::loop_now_hrtime
// @return pid of loop_run
long loop_pid(loop_t* loop);
// @return tid of loop_run
long loop_tid(loop_t* loop);

// userdata
void loop_set_userdata(loop_t* loop, void* userdata);
void* loop_userdata(loop_t* loop);

// custom_event
/*
 * event_t ev;
 * memset(&ev, 0, sizeof(event_t));
 * ev.event_type = (event_type_e)(EVENT_TYPE_CUSTOM + 1);
 * ev.cb = custom_event_cb;
 * ev.userdata = userdata;
 * loop_post_event(loop, &ev);
 */
// NOTE: loop_post_event is thread-safe, used to post event from other thread to loop thread.
void loop_post_event(loop_t* loop, event_t* ev);

// idle
idle_t* idle_add(loop_t* loop, idle_cb cb, uint32_t repeat = INFINITE);
void idle_del(idle_t* idle);

// timer
// @param timeout: unit(ms)
timer_t* timer_add(loop_t* loop, timer_cb cb, uint32_t timeout, uint32_t repeat = INFINITE);
/*
 * minute   hour    day     week    month       cb
 * 0~59     0~23    1~31    0~6     1~12
 *  30      -1      -1      -1      -1          cron.hourly
 *  30      1       -1      -1      -1          cron.daily
 *  30      1       15      -1      -1          cron.monthly
 *  30      1       -1       5      -1          cron.weekly
 *  30      1        1      -1      10          cron.yearly
 */
timer_t* timer_add_period(loop_t* loop, timer_cb cb, int8_t minute = 0, int8_t hour = -1,
                          int8_t day = -1, int8_t week = -1, int8_t month = -1,
                          uint32_t repeat = INFINITE);

void timer_del(timer_t* timer);
void timer_reset(timer_t* timer);

// io
//-----------------------low-level apis---------------------------------------
#define ARS_IO_READ 0x0001
#define ARS_IO_WRITE 0x0004
#define ARS_IO_RDWR (ARS_IO_READ | ARS_IO_WRITE)
/*
const char* io_engine() {
#ifdef EVENT_SELECT
    return  "select";
#elif defined(EVENT_POLL)
    return  "poll";
#elif defined(EVENT_EPOLL)
    return  "epoll";
#elif defined(EVENT_KQUEUE)
    return  "kqueue";
#elif defined(EVENT_IOCP)
    return  "iocp";
#elif defined(EVENT_PORT)
    return  "evport";
#else
    return  "noevent";
#endif
}
*/
const char* io_engine();

io_t* io_get(loop_t* loop, int fd);
int io_add(io_t* io, io_cb cb, int events = ARS_IO_READ);
int hio_del(io_t* io, int events = ARS_IO_RDWR);

// io_t fields
// NOTE: fd cannot be used as unique identifier, so we provide an id.
uint32_t io_id(io_t* io);
int io_fd(io_t* io);
int io_error(io_t* io);
int io_events(io_t* io);
int io_revents(io_t* io);
io_type_e io_type(io_t* io);
struct sockaddr* io_localaddr(io_t* io);
struct sockaddr* io_peeraddr(io_t* io);
void io_set_context(io_t* io, void* ctx);
void* io_context(io_t* io);
bool io_is_opened(io_t* io);
bool io_is_closed(io_t* io);

// set callbacks
void io_setcb_accept(io_t* io, accept_cb accept_cb);
void io_setcb_connect(io_t* io, connect_cb connect_cb);
void io_setcb_read(io_t* io, read_cb read_cb);
void io_setcb_write(io_t* io, write_cb write_cb);
void io_setcb_close(io_t* io, close_cb close_cb);

// some useful settings
// Enable SSL/TLS is so easy :)
int io_enable_ssl(io_t* io);
// TODO: One loop per thread, one readbuf per loop.
// But you can pass in your own readbuf instead of the default readbuf to avoid memcopy.
void io_set_readbuf(io_t* io, void* buf, size_t len);
// connect timeout => close_cb
void io_set_connect_timeout(io_t* io, int timeout_ms = ARS_IO_DEFAULT_CONNECT_TIMEOUT);
// close timeout => close_cb
void io_set_close_timeout(io_t* io, int timeout_ms = ARS_IO_DEFAULT_CLOSE_TIMEOUT);
// keepalive timeout => close_cb
void io_set_keepalive_timeout(io_t* io, int timeout_ms = ARS_IO_DEFAULT_KEEPALIVE_TIMEOUT);
/*
void send_heartbeat(io_t* io) {
    static char buf[] = "PING\r\n";
    io_write(io, buf, 6);
}
io_set_heartbeat(io, 3000, send_heartbeat);
*/
typedef void (*hio_send_heartbeat_fn)(io_t* io);
// heartbeat interval => hio_send_heartbeat_fn
void io_set_heartbeat(io_t* io, int interval_ms, hio_send_heartbeat_fn fn);

// Nonblocking, poll IO events in the loop to call corresponding callback.
// io_add(io, ARS_IO_READ) => accept => accept_cb
int io_accept(io_t* io);
// connect => io_add(io, ARS_IO_WRITE) => connect_cb
int io_connect(io_t* io);
// io_add(io, ARS_IO_READ) => read => read_cb
int io_read(io_t* io);
#define ars_io_read_start(io) ars::sdk::event::io_read(io)
#define ars_io_read_stop(io) ars::sdk::event::hio_del(io, ARS_IO_READ)
// NOTE: io_write is thread-safe, locked by recursive_mutex, allow to be called by other threads.
// hio_try_write => io_add(io, ARS_IO_WRITE) => write => write_cb
int io_write(io_t* io, const void* buf, size_t len);
// NOTE: io_close is thread-safe, if called by other thread, loop_post_event(hio_close_event).
// hio_del(io, ARS_IO_RDWR) => close => close_cb
int io_close(io_t* io);

//------------------high-level apis-------------------------------------------
// io_get -> io_set_readbuf -> io_setcb_read -> io_read
io_t* ev_read(loop_t* loop, int fd, void* buf, size_t len, read_cb read_cb);
// io_get -> io_setcb_write -> io_write
io_t* ev_write(loop_t* loop, int fd, const void* buf, size_t len, write_cb write_cb = NULL);
// io_get -> io_close
void ev_close(loop_t* loop, int fd);

// tcp
// io_get -> io_setcb_accept -> io_accept
io_t* ev_accept(loop_t* loop, int listenfd, accept_cb accept_cb);
// io_get -> io_setcb_connect -> io_connect
io_t* ev_connect(loop_t* loop, int connfd, connect_cb connect_cb);
// io_get -> io_set_readbuf -> io_setcb_read -> io_read
io_t* ev_recv(loop_t* loop, int connfd, void* buf, size_t len, read_cb read_cb);
// io_get -> io_setcb_write -> io_write
io_t* ev_send(loop_t* loop, int connfd, const void* buf, size_t len, write_cb write_cb = NULL);

// udp
void io_set_type(io_t* io, io_type_e type);
void io_set_localaddr(io_t* io, struct sockaddr* addr, int addrlen);
void io_set_peeraddr(io_t* io, struct sockaddr* addr, int addrlen);
// NOTE: must call io_set_peeraddr before ev_recvfrom/ev_sendto
// io_get -> io_set_readbuf -> io_setcb_read -> io_read
io_t* ev_recvfrom(loop_t* loop, int sockfd, void* buf, size_t len, read_cb read_cb);
// io_get -> io_setcb_write -> io_write
io_t* ev_sendto(loop_t* loop, int sockfd, const void* buf, size_t len, write_cb write_cb = NULL);

//-----------------top-level apis---------------------------------------------
// Resolver -> socket -> io_get
io_t* ev_create(loop_t* loop, const char* host, int port, int type = SOCK_STREAM);
// @tcp_server: socket -> bind -> listen -> ev_accept
// @see examples/tcp_echo_server.c
io_t* loop_create_tcp_server(loop_t* loop, const char* host, int port, accept_cb accept_cb);
// @tcp_client: ev_create(loop, host, port, SOCK_STREAM) -> ev_connect
// @see examples/nc.c
io_t* loop_create_tcp_client(loop_t* loop, const char* host, int port, connect_cb connect_cb);

// @udp_server: socket -> bind -> io_get
// @see examples/udp_echo_server.c
io_t* loop_create_udp_server(loop_t* loop, const char* host, int port);
// @udp_client: ev_create(loop, host, port, SOCK_DGRAM)
// @see examples/nc.c
io_t* loop_create_udp_client(loop_t* loop, const char* host, int port);

//-----------------upstream---------------------------------------------
// io_read(io)
// io_read(io->upstream_io)
void io_read_upstream(io_t* io);
// io_write(io->upstream_io, buf, bytes)
void io_write_upstream(io_t* io, void* buf, int bytes);
// io_close(io->upstream_io)
void io_close_upstream(io_t* io);

// io1->upstream_io = io2;
// io2->upstream_io = io1;
// io_setcb_read(io1, io_write_upstream);
// io_setcb_read(io2, io_write_upstream);
void io_setup_upstream(io_t* io1, io_t* io2);

// @return io->upstream_io
io_t* io_get_upstream(io_t* io);

// @tcp_upstream: ev_create -> io_setup_upstream -> io_setcb_close(io_close_upstream) -> ev_connect
// -> on_connect -> io_read_upstream
// @return upstream_io
// @see examples/tcp_proxy_server
io_t* io_setup_tcp_upstream(io_t* io, const char* host, int port, int ssl = 0);
#define ars_io_setup_ssl_upstream(io, host, port) ars::sdk::event::io_setup_tcp_upstream(io, host, port, 1)

// @udp_upstream: ev_create -> io_setup_upstream -> io_read_upstream
// @return upstream_io
// @see examples/udp_proxy_server
io_t* io_setup_udp_upstream(io_t* io, const char* host, int port);

}  // namespace event

}  // namespace sdk

}  // namespace ars
