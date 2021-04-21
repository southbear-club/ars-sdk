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
 * @file TcpServer.hpp
 * @brief 
 * @author wotsen (astralrovers@outlook.com)
 * @version 1.0.0
 * @date 2021-04-18
 * 
 * @copyright MIT
 * 
 */
#pragma once

#include "aru/sdk/net/sock.hpp"
#include "aru/sdk/net/ssl.hpp"

#include "EventLoopThreadPool.hpp"
#include "Callback.hpp"
#include "Channel.hpp"

namespace aru {
    
namespace evpp {

class TcpServer {
public:
    TcpServer() {
        listenfd = -1;
        tls = false;
        max_connections = 0xFFFFFFFF;
    }

    virtual ~TcpServer() {
    }

    //@retval >=0 listenfd, <0 error
    int createsocket(int port, const char* host = "0.0.0.0") {
        listenfd = sdk::sock_fast_listen(port, host);
        return listenfd;
    }

    void setMaxConnectionNum(uint32_t num) {
        max_connections = num;
    }
    void setThreadNum(int num) {
        loop_threads.setThreadNum(num);
    }
    void start(bool wait_threads_started = true) {
        loop_threads.start(wait_threads_started, [this](const EventLoopPtr& loop){
            assert(listenfd >= 0);
            sdk::event::io_t* listenio = sdk::event::ev_accept(loop->loop(), listenfd, onAccept);
            aru_event_set_userdata(listenio, this);
            if (tls) {
                sdk::event::io_enable_ssl(listenio);
            }
        });
    }
    void stop(bool wait_threads_stopped = true) {
        loop_threads.stop(wait_threads_stopped);
    }

    EventLoopPtr loop(int idx = -1) {
        return loop_threads.loop(idx);
    }
    sdk::event::loop_t* hloop(int idx = -1) {
        return loop_threads.hloop(idx);
    }

    int withTLS(const char* cert_file, const char* key_file) {
        tls = true;
        if (cert_file) {
            sdk::ssl_ctx_init_param_t param;
            memset(&param, 0, sizeof(param));
            param.crt_file = cert_file;
            param.key_file = key_file;
            param.endpoint = 0;
            return sdk::ssl_ctx_init(&param) == NULL ? -1 : 0;
        }
        return 0;
    }

    // channel
    const SocketChannelPtr& addChannel(sdk::event::io_t* io) {
        std::lock_guard<std::mutex> locker(mutex_);
        int fd = sdk::event::io_fd(io);
        channels[fd] = SocketChannelPtr(new SocketChannel(io));
        return channels[fd];
    }

    void removeChannel(const SocketChannelPtr& channel) {
        std::lock_guard<std::mutex> locker(mutex_);
        int fd = channel->fd();
        channels.erase(fd);
    }

    size_t connectionNum() {
        std::lock_guard<std::mutex> locker(mutex_);
        return channels.size();
    }

private:
    static void onAccept(sdk::event::io_t* connio) {
        TcpServer* server = (TcpServer*)aru_event_userdata(connio);
        if (server->connectionNum() >= server->max_connections) {
            // hlogw("over max_connections");
            sdk::event::io_close(connio);
            return;
        }
        const SocketChannelPtr& channel = server->addChannel(connio);
        channel->status = SocketChannel::CONNECTED;

        channel->onread = [server, &channel](Buffer* buf) {
            if (server->onMessage) {
                server->onMessage(channel, buf);
            }
        };
        channel->onwrite = [server, &channel](Buffer* buf) {
            if (server->onWriteComplete) {
                server->onWriteComplete(channel, buf);
            }
        };
        channel->onclose = [server, &channel]() {
            channel->status = SocketChannel::CLOSED;
            if (server->onConnection) {
                server->onConnection(channel);
            }
            server->removeChannel(channel);
            // NOTE: After removeChannel, channel may be destroyed,
            // so in this lambda function, no code should be added below.
        };

        channel->startRead();
        if (server->onConnection) {
            server->onConnection(channel);
        }
    }

public:
    int                     listenfd;
    bool                    tls;
    // Callback
    ConnectionCallback      onConnection;
    MessageCallback         onMessage;
    WriteCompleteCallback   onWriteComplete;

    uint32_t                max_connections;

private:
    EventLoopThreadPool     loop_threads;
    // fd => SocketChannelPtr
    std::map<int, SocketChannelPtr> channels; // GUAREDE_BY(mutex_)
    std::mutex                      mutex_;
};

} // namespace evpp

} // namespace aru
