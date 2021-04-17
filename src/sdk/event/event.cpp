#include "aru/sdk/event/event.hpp"
#include <atomic>
#include "aru/sdk/atomic/atomic.hpp"
#include "aru/sdk/macros/attr.hpp"
#include "aru/sdk/memory/mem.hpp"
#include "aru/sdk/net/sock.hpp"

namespace aru {

namespace sdk {

namespace event {

uint64_t loop_next_event_id() {
    static std::atomic<int> s_id(0);
    return ++s_id;
}

uint32_t io_next_id() {
    static std::atomic<int> s_id(0);
    return ++s_id;
}

uint32_t io_id(io_t* io) { return io->id; }

int io_fd(io_t* io) { return io->fd; }

io_type_e io_type(io_t* io) { return io->io_type; }

int io_error(io_t* io) { return io->error; }

int io_events(io_t* io) { return io->events; }

int io_revents(io_t* io) { return io->revents; }

struct sockaddr* io_localaddr(io_t* io) {
    return io->localaddr;
}

struct sockaddr* io_peeraddr(io_t* io) {
    return io->peeraddr;
}

void io_set_context(io_t* io, void* ctx) { io->ctx = ctx; }

void* io_context(io_t* io) { return io->ctx; }

void io_setcb_accept(io_t* io, accept_cb accept_cb) { io->accept_cb = accept_cb; }

void io_setcb_connect(io_t* io, connect_cb connect_cb) { io->connect_cb = connect_cb; }

void io_setcb_read(io_t* io, read_cb read_cb) { io->read_cb = read_cb; }

void io_setcb_write(io_t* io, write_cb write_cb) { io->write_cb = write_cb; }

void io_setcb_close(io_t* io, close_cb close_cb) { io->close_cb = close_cb; }

void io_set_type(io_t* io, io_type_e type) { io->io_type = type; }

void io_set_localaddr(io_t* io, struct sockaddr* addr, int addrlen) {
    if (io->localaddr == NULL) {
        ARU_ALLOC(io->localaddr, sizeof(aru::sdk::sock_addr_t));
    }
    memcpy(io->localaddr, addr, addrlen);
}

void io_set_peeraddr(io_t* io, struct sockaddr* addr, int addrlen) {
    if (io->peeraddr == NULL) {
        ARU_ALLOC(io->peeraddr, sizeof(aru::sdk::sock_addr_t));
    }
    memcpy(io->peeraddr, addr, addrlen);
}

int io_enable_ssl(io_t* io) {
    io->io_type = IO_TYPE_SSL;
    return 0;
}

void io_set_readbuf(io_t* io, void* buf, size_t len) {
    if (buf == NULL || len == 0) {
        loop_t* loop = io->loop;
        if (loop && (loop->readbuf.base == NULL || loop->readbuf.len == 0)) {
            loop->readbuf.len = ARU_LOOP_READ_BUFSIZE;
            ARU_ALLOC(loop->readbuf.base, loop->readbuf.len);
            io->readbuf = loop->readbuf;
        }
    } else {
        io->readbuf.base = (char*)buf;
        io->readbuf.len = len;
    }
}

void io_set_connect_timeout(io_t* io, int timeout_ms) { io->connect_timeout = timeout_ms; }

void io_set_close_timeout(io_t* io, int timeout_ms) { io->close_timeout = timeout_ms; }

static void __keepalive_timeout_cb(timer_t* timer) {
    io_t* io = (io_t*)timer->privdata;
    if (io) {
        char ARU_UNUSED(localaddrstr[ARU_SOCKADDR_STRLEN]) = {0};
        char ARU_UNUSED(peeraddrstr[ARU_SOCKADDR_STRLEN]) = {0};
        // hlogw("keepalive timeout [%s] <=> [%s]",
        //         SOCKADDR_STR(io->localaddr, localaddrstr),
        //         SOCKADDR_STR(io->peeraddr, peeraddrstr));
        io->error = ETIMEDOUT;
        io_close(io);
    }
}

void io_set_keepalive_timeout(io_t* io, int timeout_ms) {
    if (io->keepalive_timer) {
        if (timeout_ms == 0) {
            timer_del(io->keepalive_timer);
            io->keepalive_timer = NULL;
        } else {
            ((struct timeout_s*)io->keepalive_timer)->timeout = timeout_ms;
            timer_reset(io->keepalive_timer);
        }
    } else {
        io->keepalive_timer = timer_add(io->loop, __keepalive_timeout_cb, timeout_ms, 1);
        io->keepalive_timer->privdata = io;
    }
    io->keepalive_timeout = timeout_ms;
}

static void __heartbeat_timer_cb(timer_t* timer) {
    io_t* io = (io_t*)timer->privdata;
    if (io && io->heartbeat_fn) {
        io->heartbeat_fn(io);
    }
}

void io_set_heartbeat(io_t* io, int interval_ms, hio_send_heartbeat_fn fn) {
    if (io->heartbeat_timer) {
        if (interval_ms == 0) {
            timer_del(io->heartbeat_timer);
            io->heartbeat_timer = NULL;
        } else {
            ((struct timeout_s*)io->heartbeat_fn)->timeout = interval_ms;
            timer_reset(io->keepalive_timer);
        }
    } else {
        io->heartbeat_timer = timer_add(io->loop, __heartbeat_timer_cb, interval_ms, INFINITE);
        io->heartbeat_timer->privdata = io;
    }
    io->heartbeat_interval = interval_ms;
    io->heartbeat_fn = fn;
}

}  // namespace event

}  // namespace sdk

}  // namespace aru
