#include "aru/sdk/event/iowatcher.hpp"
#ifndef EVENT_IOCP
#include <unistd.h>
#include "aru/sdk/event/event.hpp"
#include "aru/sdk/macros/attr.hpp"
#include "aru/sdk/net/sock.hpp"
#include "aru/sdk/net/ssl.hpp"
#include "aru/sdk/thread/thread.hpp"

namespace aru {

namespace sdk {

namespace event {

static void __connect_timeout_cb(timer_t* timer) {
    io_t* io = (io_t*)timer->privdata;
    if (io) {
        char ARU_UNUSED(localaddrstr[ARU_SOCKADDR_STRLEN]) = {0};
        char ARU_UNUSED(peeraddrstr[ARU_SOCKADDR_STRLEN]) = {0};
        // hlogw("connect timeout [%s] <=> [%s]",
        //         SOCKADDR_STR(io->localaddr, localaddrstr),
        //         SOCKADDR_STR(io->peeraddr, peeraddrstr));
        io->error = ETIMEDOUT;
        io_close(io);
    }
}

static void __close_timeout_cb(timer_t* timer) {
    io_t* io = (io_t*)timer->privdata;
    if (io) {
        char ARU_UNUSED(localaddrstr[ARU_SOCKADDR_STRLEN]) = {0};
        char ARU_UNUSED(peeraddrstr[ARU_SOCKADDR_STRLEN]) = {0};
        // hlogw("close timeout [%s] <=> [%s]",
        //         SOCKADDR_STR(io->localaddr, localaddrstr),
        //         SOCKADDR_STR(io->peeraddr, peeraddrstr));
        io->error = ETIMEDOUT;
        io_close(io);
    }
}

static void __accept_cb(io_t* io) {
    /*
    char localaddrstr[SOCKADDR_STRLEN] = {0};
    char peeraddrstr[SOCKADDR_STRLEN] = {0};
    printd("accept connfd=%d [%s] <= [%s]\n", io->fd,
            SOCKADDR_STR(io->localaddr, localaddrstr),
            SOCKADDR_STR(io->peeraddr, peeraddrstr));
    */

    if (io->accept_cb) {
        // printd("accept_cb------\n");
        io->accept_cb(io);
        // printd("accept_cb======\n");
    }
}

static void __connect_cb(io_t* io) {
    /*
    char localaddrstr[SOCKADDR_STRLEN] = {0};
    char peeraddrstr[SOCKADDR_STRLEN] = {0};
    printd("connect connfd=%d [%s] => [%s]\n", io->fd,
            SOCKADDR_STR(io->localaddr, localaddrstr),
            SOCKADDR_STR(io->peeraddr, peeraddrstr));
    */
    if (io->connect_timer) {
        timer_del(io->connect_timer);
        io->connect_timer = NULL;
        io->connect_timeout = 0;
    }

    if (io->connect_cb) {
        // printd("connect_cb------\n");
        io->connect_cb(io);
        // printd("connect_cb======\n");
    }
}

static void __read_cb(io_t* io, void* buf, int readbytes) {
    // printd("> %.*s\n", readbytes, buf);
    if (io->keepalive_timer) {
        timer_reset(io->keepalive_timer);
    }

    if (io->read_cb) {
        // printd("read_cb------\n");
        io->read_cb(io, buf, readbytes);
        // printd("read_cb======\n");
    }
}

static void __write_cb(io_t* io, const void* buf, int writebytes) {
    // printd("< %.*s\n", writebytes, buf);
    if (io->keepalive_timer) {
        timer_reset(io->keepalive_timer);
    }

    if (io->write_cb) {
        // printd("write_cb------\n");
        io->write_cb(io, buf, writebytes);
        // printd("write_cb======\n");
    }
}

static void __close_cb(io_t* io) {
    // printd("close fd=%d\n", io->fd);
    if (io->connect_timer) {
        timer_del(io->connect_timer);
        io->connect_timer = NULL;
        io->connect_timeout = 0;
    }

    if (io->close_timer) {
        timer_del(io->close_timer);
        io->close_timer = NULL;
        io->close_timeout = 0;
    }

    if (io->keepalive_timer) {
        timer_del(io->keepalive_timer);
        io->keepalive_timer = NULL;
        io->keepalive_timeout = 0;
    }

    if (io->heartbeat_timer) {
        timer_del(io->heartbeat_timer);
        io->heartbeat_timer = NULL;
        io->heartbeat_interval = 0;
        io->heartbeat_fn = NULL;
    }

    if (io->close_cb) {
        // printd("close_cb------\n");
        io->close_cb(io);
        // printd("close_cb======\n");
    }
}

static void ssl_server_handshark(io_t* io) {
    printd("ssl server handshark...\n");
    int ret = ssl_accept(io->ssl);
    if (ret == 0) {
        // handshark finish
        iowatcher_del_event(io->loop, io->fd, ARU_IO_READ);
        io->events &= ~ARU_IO_READ;
        io->cb = NULL;
        printd("ssl handshark finished.\n");
        __accept_cb(io);
    } else if (ret == SSL_WANT_READ) {
        if ((io->events & ARU_IO_READ) == 0) {
            io_add(io, ssl_server_handshark, ARU_IO_READ);
        }
    } else {
        // hloge("ssl handshake failed: %d", ret);
        io_close(io);
    }
}

static void ssl_client_handshark(io_t* io) {
    printd("ssl client handshark...\n");
    int ret = ssl_connect(io->ssl);
    if (ret == 0) {
        // handshark finish
        iowatcher_del_event(io->loop, io->fd, ARU_IO_READ);
        io->events &= ~ARU_IO_READ;
        io->cb = NULL;
        printd("ssl handshark finished.\n");
        __connect_cb(io);
    } else if (ret == SSL_WANT_READ) {
        if ((io->events & ARU_IO_READ) == 0) {
            io_add(io, ssl_client_handshark, ARU_IO_READ);
        }
    } else {
        // hloge("ssl handshake failed: %d", ret);
        io_close(io);
    }
}

static void nio_accept(io_t* io) {
    // printd("nio_accept listenfd=%d\n", io->fd);
    socklen_t addrlen;
accept:
    addrlen = sizeof(sock_addr_t);
    int connfd = accept(io->fd, io->peeraddr, &addrlen);
    io_t* connio = NULL;
    if (connfd < 0) {
        if (socket_errno() == EAGAIN) {
            // goto accept_done;
            return;
        } else {
            io->error = socket_errno();
            perror("accept");
            goto accept_error;
        }
    }
    addrlen = sizeof(sock_addr_t);
    getsockname(connfd, io->localaddr, &addrlen);
    connio = io_get(io->loop, connfd);
    // NOTE: inherit from listenio
    connio->accept_cb = io->accept_cb;
    connio->userdata = io->userdata;

    if (io->io_type == IO_TYPE_SSL) {
        ssl_ctx_t ssl_ctx = ssl_ctx_instance();
        if (ssl_ctx == NULL) {
            goto accept_error;
        }
        ssl_t ssl = ssl_new(ssl_ctx, connfd);
        if (ssl == NULL) {
            goto accept_error;
        }
        io_enable_ssl(connio);
        connio->ssl = ssl;
        ssl_server_handshark(connio);
    } else {
        // NOTE: SSL call accept_cb after handshark finished
        __accept_cb(connio);
    }

    goto accept;

accept_error:
    io_close(io);
}

static void nio_connect(io_t* io) {
    // printd("nio_connect connfd=%d\n", io->fd);
    socklen_t addrlen = sizeof(sock_addr_t);
    int ret = getpeername(io->fd, io->peeraddr, &addrlen);
    if (ret < 0) {
        io->error = socket_errno();
        printd("connect failed: %s: %d\n", strerror(socket_errno()), socket_errno());
        goto connect_failed;
    } else {
        addrlen = sizeof(sock_addr_t);
        getsockname(io->fd, io->localaddr, &addrlen);

        if (io->io_type == IO_TYPE_SSL) {
            ssl_ctx_t ssl_ctx = ssl_ctx_instance();
            if (ssl_ctx == NULL) {
                goto connect_failed;
            }
            ssl_t ssl = ssl_new(ssl_ctx, io->fd);
            if (ssl == NULL) {
                goto connect_failed;
            }
            io->ssl = ssl;
            ssl_client_handshark(io);
        } else {
            // NOTE: SSL call connect_cb after handshark finished
            __connect_cb(io);
        }

        return;
    }

connect_failed:
    io_close(io);
}

static int __nio_read(io_t* io, void* buf, int len) {
    int nread = 0;
    switch (io->io_type) {
        case IO_TYPE_SSL:
            nread = ssl_read(io->ssl, buf, len);
            break;
        case IO_TYPE_TCP:
#ifdef OS_UNIX
            nread = read(io->fd, buf, len);
#else
            nread = recv(io->fd, buf, len, 0);
#endif
            break;
        case IO_TYPE_UDP:
        case IO_TYPE_IP: {
            socklen_t addrlen = sizeof(sock_addr_t);
            nread = recvfrom(io->fd, buf, len, 0, io->peeraddr, &addrlen);
        } break;
        default:
            nread = read(io->fd, buf, len);
            break;
    }
    return nread;
}

static int __nio_write(io_t* io, const void* buf, int len) {
    int nwrite = 0;
    switch (io->io_type) {
        case IO_TYPE_SSL:
            nwrite = ssl_write(io->ssl, buf, len);
            break;
        case IO_TYPE_TCP:
#ifdef OS_UNIX
            nwrite = write(io->fd, buf, len);
#else
            nwrite = send(io->fd, buf, len, 0);
#endif
            break;
        case IO_TYPE_UDP:
        case IO_TYPE_IP:
            nwrite = sendto(io->fd, buf, len, 0, io->peeraddr, ARU_SOCKADDR_LEN(io->peeraddr));
            break;
        default:
            nwrite = write(io->fd, buf, len);
            break;
    }
    return nwrite;
}

static void nio_read(io_t* io) {
    // printd("nio_read fd=%d\n", io->fd);
    void* buf;
    int len, nread;
read:
    if (io->readbuf.base == NULL || io->readbuf.len == 0) {
        io_set_readbuf(io, io->loop->readbuf.base, io->loop->readbuf.len);
    }
    buf = io->readbuf.base;
    len = io->readbuf.len;
    nread = __nio_read(io, buf, len);
    // printd("read retval=%d\n", nread);
    if (nread < 0) {
        if (socket_errno() == EAGAIN) {
            // goto read_done;
            return;
        } else {
            io->error = socket_errno();
            // perror("read");
            goto read_error;
        }
    }
    if (nread == 0) {
        goto disconnect;
    }
    __read_cb(io, buf, nread);
    if (nread == len) {
        goto read;
    }
    return;
read_error:
disconnect:
    io_close(io);
}

static void nio_write(io_t* io) {
    // printd("nio_write fd=%d\n", io->fd);
    int nwrite = 0;
    mutex_lock(&io->write_mutex);
write:
    if (write_queue_empty(&io->write_queue)) {
        mutex_unlock(&io->write_mutex);
        if (io->close) {
            io->close = 0;
            io_close(io);
        }
        return;
    }
    offset_buf_t* pbuf = write_queue_front(&io->write_queue);
    char* buf = pbuf->base + pbuf->offset;
    int len = pbuf->len - pbuf->offset;
    nwrite = __nio_write(io, buf, len);
    // printd("write retval=%d\n", nwrite);
    if (nwrite < 0) {
        if (socket_errno() == EAGAIN) {
            // goto write_done;
            mutex_unlock(&io->write_mutex);
            return;
        } else {
            io->error = socket_errno();
            // perror("write");
            goto write_error;
        }
    }
    if (nwrite == 0) {
        goto disconnect;
    }
    __write_cb(io, buf, nwrite);
    pbuf->offset += nwrite;
    if (nwrite == len) {
        ARU_FREE(pbuf->base);
        write_queue_pop_front(&io->write_queue);
        // write next
        goto write;
    }
    mutex_unlock(&io->write_mutex);
    return;
write_error:
disconnect:
    mutex_unlock(&io->write_mutex);
    io_close(io);
}

static void hio_handle_events(io_t* io) {
    if ((io->events & ARU_IO_READ) && (io->revents & ARU_IO_READ)) {
        if (io->accept) {
            nio_accept(io);
        } else {
            nio_read(io);
        }
    }

    if ((io->events & ARU_IO_WRITE) && (io->revents & ARU_IO_WRITE)) {
        // NOTE: del ARU_IO_WRITE, if write_queue empty
        mutex_lock(&io->write_mutex);
        if (write_queue_empty(&io->write_queue)) {
            iowatcher_del_event(io->loop, io->fd, ARU_IO_WRITE);
            io->events &= ~ARU_IO_WRITE;
        }
        mutex_unlock(&io->write_mutex);
        if (io->connect) {
            // NOTE: connect just do once
            // ONESHOT
            io->connect = 0;

            nio_connect(io);
        } else {
            nio_write(io);
        }
    }

    io->revents = 0;
}

int io_accept(io_t* io) {
    io->accept = 1;
    io_add(io, hio_handle_events, ARU_IO_READ);
    return 0;
}

int io_connect(io_t* io) {
    int ret = connect(io->fd, io->peeraddr, ARU_SOCKADDR_LEN(io->peeraddr));
#ifdef OS_WIN
    if (ret < 0 && socket_errno() != WSAEWOULDBLOCK) {
#else
    if (ret < 0 && socket_errno() != EINPROGRESS) {
#endif
        perror("connect");
        io_close(io);
        return ret;
    }
    if (ret == 0) {
        // connect ok
        __connect_cb(io);
        return 0;
    }
    int timeout = io->connect_timeout ? io->connect_timeout : ARU_IO_DEFAULT_CONNECT_TIMEOUT;
    io->connect_timer = timer_add(io->loop, __connect_timeout_cb, timeout, 1);
    io->connect_timer->privdata = io;
    io->connect = 1;
    return io_add(io, hio_handle_events, ARU_IO_WRITE);
}

int io_read(io_t* io) {
    if (io->closed) {
        // hloge("io_read called but fd[%d] already closed!", io->fd);
        return -1;
    }
    return io_add(io, hio_handle_events, ARU_IO_READ);
}

int io_write(io_t* io, const void* buf, size_t len) {
    if (io->closed) {
        // hloge("io_write called but fd[%d] already closed!", io->fd);
        return -1;
    }
    int nwrite = 0;
    mutex_lock(&io->write_mutex);
    if (write_queue_empty(&io->write_queue)) {
        // try_write:
        nwrite = __nio_write(io, buf, len);
        // printd("write retval=%d\n", nwrite);
        if (nwrite < 0) {
            if (socket_errno() == EAGAIN) {
                nwrite = 0;
                // hlogw("try_write failed, enqueue!");
                goto enqueue;
            } else {
                // perror("write");
                io->error = socket_errno();
                goto write_error;
            }
        }
        if (nwrite == 0) {
            goto disconnect;
        }
        __write_cb(io, buf, nwrite);
        if (nwrite == (int)len) {
            // goto write_done;
            mutex_unlock(&io->write_mutex);
            return nwrite;
        }
    enqueue:
        io_add(io, hio_handle_events, ARU_IO_WRITE);
    }
    if (nwrite < (int)len) {
        offset_buf_t rest;
        rest.len = len;
        rest.offset = nwrite;
        // NOTE: free in nio_write
        ARU_ALLOC(rest.base, rest.len);
        memcpy(rest.base, buf, rest.len);
        if (io->write_queue.maxsize == 0) {
            write_queue_init(&io->write_queue, 4);
        }
        write_queue_push_back(&io->write_queue, &rest);
    }
    mutex_unlock(&io->write_mutex);
    return nwrite;
write_error:
disconnect:
    mutex_unlock(&io->write_mutex);
    io_close(io);
    return nwrite;
}

static void hio_close_event_cb(event_t* ev) {
    io_t* io = (io_t*)ev->userdata;
    uint32_t id = (uintptr_t)ev->privdata;
    if (io->id == id) {
        io_close((io_t*)ev->userdata);
    }
}

int io_close(io_t* io) {
    if (io->closed) return 0;
    if ((long)gettid() != io->loop->tid) {
        event_t ev;
        memset(&ev, 0, sizeof(ev));
        ev.cb = hio_close_event_cb;
        ev.userdata = io;
        ev.privdata = (void*)(uintptr_t)io->id;
        loop_post_event(io->loop, &ev);
        return 0;
    }
    mutex_lock(&io->write_mutex);
    if (!write_queue_empty(&io->write_queue) && io->error == 0 && io->close == 0) {
        mutex_unlock(&io->write_mutex);
        io->close = 1;
        // hlogw("write_queue not empty, close later.");
        int timeout_ms = io->close_timeout ? io->close_timeout : ARU_IO_DEFAULT_CLOSE_TIMEOUT;
        io->close_timer = timer_add(io->loop, __close_timeout_cb, timeout_ms, 1);
        io->close_timer->privdata = io;
        return 0;
    }
    mutex_unlock(&io->write_mutex);

    io->closed = 1;
    io_done(io);
    __close_cb(io);
    if (io->ssl) {
        ssl_free(io->ssl);
        io->ssl = NULL;
    }
    if (io->io_type & IO_TYPE_SOCKET) {
        close(io->fd);
    }
    return 0;
}

}  // namespace event

}  // namespace sdk

}  // namespace aru

#endif
