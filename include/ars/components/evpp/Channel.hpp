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
 * @file Channel.hpp
 * @brief 
 * @author wotsen (astralrovers@outlook.com)
 * @version 1.0.0
 * @date 2021-04-18
 * 
 * @copyright MIT
 * 
 */
#pragma once

#include <string>
#include <functional>
#include <memory>

#include "ars/sdk/event/loop.hpp"
#include "ars/sdk/net/sock.hpp"
#include "Buffer.hpp"

namespace ars {
    
namespace evpp {

class Channel {
public:
    Channel(sdk::event::io_t* io = NULL) {
        io_ = io;
        fd_ = -1;
        id_ = 0;
        ctx_ = NULL;
        if (io) {
            fd_ = sdk::event::io_fd(io);
            id_ = sdk::event::io_id(io);
            sdk::event::io_set_context(io, this);
            sdk::event::io_setcb_read(io_, on_read);
            sdk::event::io_setcb_write(io_, on_write);
            sdk::event::io_setcb_close(io_, on_close);
        }
        status = isOpened() ? OPENED : CLOSED;
    }

    virtual ~Channel() {
        close();
    }

    sdk::event::io_t*      io() { return io_; }
    int         fd() { return fd_; }
    uint32_t    id() { return id_; }
    int error() { return sdk::event::io_error(io_); }

    // context
    void* context() {
        return ctx_;
    }
    void setContext(void* ctx) {
        ctx_ = ctx;
    }
    template<class T>
    T* newContext() {
        ctx_ = new T;
        return (T*)ctx_;
    }
    template<class T>
    T* getContext() {
        return (T*)ctx_;
    }
    template<class T>
    void deleteContext() {
        if (ctx_) {
            delete (T*)ctx_;
            ctx_ = NULL;
        }
    }

    bool isOpened() {
        if (io_ == NULL) return false;
        return id_ == sdk::event::io_id(io_) && sdk::event::io_is_opened(io_);
    }
    bool isClosed() {
        return !isOpened();
    }

    int startRead() {
        if (!isOpened()) return 0;
        return aru_io_read_start(io_);
    }

    int stopRead() {
        if (!isOpened()) return 0;
        return aru_io_read_stop(io_);
    }

    int write(const void* data, int size) {
        if (!isOpened()) return 0;
        return sdk::event::io_write(io_, data, size);
    }

    int write(Buffer* buf) {
        return write(buf->data(), buf->size());
    }

    int write(const std::string& str) {
        return write(str.data(), str.size());
    }

    int close() {
        if (!isOpened()) return 0;
        return sdk::event::io_close(io_);
    }

public:
    sdk::event::io_t*      io_;
    int         fd_;
    uint32_t    id_;
    void*       ctx_;
    enum Status {
        // Channel::Status
        OPENED,
        CLOSED,
        // SocketChannel::Status
        CONNECTING,
        CONNECTED,
        DISCONNECTED,
    } status;
    std::function<void(Buffer*)> onread;
    std::function<void(Buffer*)> onwrite;
    std::function<void()>        onclose;

private:
    static void on_read(sdk::event::io_t* io, void* data, int readbytes) {
        Channel* channel = (Channel*)sdk::event::io_context(io);
        if (channel && channel->onread) {
            Buffer buf(data, readbytes);
            channel->onread(&buf);
        }
    }

    static void on_write(sdk::event::io_t* io, const void* data, int writebytes) {
        Channel* channel = (Channel*)sdk::event::io_context(io);
        if (channel && channel->onwrite) {
            Buffer buf((void*)data, writebytes);
            channel->onwrite(&buf);
        }
    }

    static void on_close(sdk::event::io_t* io) {
        Channel* channel = (Channel*)sdk::event::io_context(io);
        if (channel) {
            channel->status = CLOSED;
            if (channel->onclose) {
                channel->onclose();
            }
        }
    }
};

class SocketChannel : public Channel {
public:
    // for TcpClient
    std::function<void()>   onconnect;

    SocketChannel(sdk::event::io_t* io) : Channel(io) {
    }
    virtual ~SocketChannel() {}

    int enableSSL() {
        return sdk::event::io_enable_ssl(io_);
    }

    void setConnectTimeout(int timeout_ms) {
        sdk::event::io_set_connect_timeout(io_, timeout_ms);
    }

    int startConnect(int port, const char* host = "127.0.0.1") {
        sdk::sock_addr_t peeraddr;
        memset(&peeraddr, 0, sizeof(peeraddr));
        int ret = sdk::sock_set_ipport(&peeraddr, host, port);
        if (ret != 0) {
            // hloge("unknown host %s", host);
            return ret;
        }
        return startConnect(&peeraddr.sa);
    }

    int startConnect(struct sockaddr* peeraddr) {
        sdk::event::io_set_peeraddr(io_, peeraddr, ARS_SOCKADDR_LEN(peeraddr));
        return startConnect();
    }

    int startConnect() {
        status = CONNECTING;
        sdk::event::io_setcb_connect(io_, on_connect);
        return sdk::event::io_connect(io_);
    }

    bool isConnected() {
        return isOpened() && status == CONNECTED;
    }

    std::string localaddr() {
        struct sockaddr* addr = sdk::event::io_localaddr(io_);
        char buf[ARS_SOCKADDR_STRLEN] = {0};
        return ARS_SOCKADDR_STR(addr, buf);
    }

    std::string peeraddr() {
        struct sockaddr* addr = sdk::event::io_peeraddr(io_);
        char buf[ARS_SOCKADDR_STRLEN] = {0};
        return ARS_SOCKADDR_STR(addr, buf);
    }

    int send(const std::string& str) {
        return write(str);
    }

private:
    static void on_connect(sdk::event::io_t* io) {
        SocketChannel* channel = (SocketChannel*)sdk::event::io_context(io);
        if (channel) {
            channel->status = CONNECTED;
            if (channel->onconnect) {
                channel->onconnect();
            }
        }
    }
};

typedef std::shared_ptr<Channel>        ChannelPtr;
typedef std::shared_ptr<SocketChannel>  SocketChannelPtr;

} // namespace evpp

} // namespace ars
