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
 * @file UdpClient.hpp
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

#include "EventLoopThread.hpp"
#include "Callback.hpp"
#include "Channel.hpp"

namespace aru {
    
namespace evpp {

class UdpClient {
public:
    UdpClient() {
    }

    virtual ~UdpClient() {
    }

    EventLoopPtr loop() {
        return loop_thread.loop();
    }

    //@retval >=0 sockfd, <0 error
    int createsocket(int port, const char* host = "127.0.0.1") {
        sdk::event::io_t* io = sdk::event::loop_create_udp_client(loop_thread.hloop(), host, port);
        if (io == NULL) return -1;
        channel.reset(new SocketChannel(io));
        return channel->fd();
    }

    void start(bool wait_threads_started = true) {
        loop_thread.start(wait_threads_started,
            [this]() {
                assert(channel != NULL);
                channel->onread = [this](Buffer* buf) {
                    if (onMessage) {
                        onMessage(channel, buf);
                    }
                };
                channel->onwrite = [this](Buffer* buf) {
                    if (onWriteComplete) {
                        onWriteComplete(channel, buf);
                    }
                };
                channel->startRead();
                return 0;
            }
        );
    }
    void stop(bool wait_threads_stopped = true) {
        loop_thread.stop(wait_threads_stopped);
    }

    int sendto(Buffer* buf) {
        if (channel == NULL) return 0;
        return channel->write(buf);
    }

    int sendto(const std::string& str) {
        if (channel == NULL) return 0;
        return channel->write(str);
    }

public:
    SocketChannelPtr        channel;
    // Callback
    MessageCallback         onMessage;
    WriteCompleteCallback   onWriteComplete;
private:
    EventLoopThread         loop_thread;
};

} // namespace evpp

} // namespace aru
