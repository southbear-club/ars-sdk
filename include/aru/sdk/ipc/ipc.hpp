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
 * @file ipc.hpp
 * @brief 
 * @author  ()
 * @version 1.0.0
 * @date 2021-04-10
 * 
 * @copyright MIT
 * 
 */
#pragma once
#include <stdint.h>
#include <stddef.h>
#include <semaphore.h>
#include <pthread.h>

namespace aru {
    
namespace sdk {

typedef enum ipc_role {
    IPC_SERVER = 0,
    IPC_CLIENT = 1,
} ipc_role;

struct ipc;

typedef int (*ipc_callback)(struct ipc *ipc,
                void *in_arg, size_t in_len,
                void *out_arg, size_t *out_len);

typedef struct ipc_handler {
    uint32_t func_id;
    ipc_callback cb;
} ipc_handler_t;

typedef struct ipc_header {
    uint32_t func_id;
    uint64_t time_stamp;
    uint32_t payload_len;
} ipc_header_t;

typedef struct ipc_packet {
    struct ipc_header header;
    uint8_t payload[0];
} ipc_packet_t;

typedef int (ipc_recv_cb)(struct ipc *ipc, void *buf, size_t len);
struct ipc_ops {
    void *(*init)(struct ipc *ipc, uint16_t port, enum ipc_role role);
    void (*deinit)(struct ipc *ipc);
    int (*accept)(struct ipc *ipc);
    int (*connect)(struct ipc *ipc, const char *name);
    int (*register_recv_cb)(struct ipc *i, ipc_recv_cb cb);
    int (*send)(struct ipc *i, const void *buf, size_t len);
    int (*recv)(struct ipc *i, void *buf, size_t len);
    int (*unicast)();//TODO
    int (*broadcast)();//TODO
};

typedef struct ipc {
    void *ctx;
    int fd;
    int afd;
    enum ipc_role role;
    struct ipc_packet packet;
    const struct ipc_ops *ops;
    sem_t sem;
    void *resp_buf;//async response buffer;
    int resp_len;
    // dict *async_cmd_list;
    pthread_t tid;
    // struct gevent_base *evbase;
} ipc_t;

} // namespace sdk

} // namespace aru
