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
 * @file sock.hpp
 * @brief 
 * @author wotsen (astralrovers@outlook.com)
 * @version 1.0.0
 * @date 2021-04-04
 * 
 * @copyright MIT
 * 
 */
#pragma once

#include <time.h>
#include <string>

namespace aru {

namespace sdk {

/**
 * @brief 网络传输协议
 */
typedef enum {
    sock_trans_tcp = 0,     ///< tcp协议
    sock_trans_udp,         ///< udp协议
    sock_trans_raw,         ///< raw
    sock_trans_invalid = 255,     ///< 无效
} sock_type_e;

/**
 * @brief ip版本
 */
typedef enum {
    sock_ip_v4 = 0,             ///< ipv4
    sock_ip_v6,                 ///< ipv6
    sock_ip_version_invalid = 255,    ///< 无效
} sock_ip_version_e;

typedef enum {
    sock_domain_local = 0,
    sock_domain_unix,           ///< unix域套接字
    sock_domain_inet,           ///< ipv4
    sock_domain_route,
    sock_domain_key,
    sock_domain_inet6,          ///< ipv6
    sock_domain_system,
    sock_domain_ndrv,

    sock_domain_invalid = 255,
} sock_domain_e;

typedef enum {
    sock_ip_protocol_ = 0,
} sock_protocol_e;

/**
 * @brief 套接字地址
 */
typedef struct {
    std::string ip;     ///< ip地址
    int port;           ///< 端口
    sock_domain_e type; ///< 域类型
} sock_addr_t;

/**
 * @brief 链接
 */
typedef struct {
    int fd;                         ///< 描述符
    sock_addr_t local;              ///< 本地
    sock_addr_t remote;             ///< 远端地址
    sock_protocol_e ip_version;  ///< ip版本
    sock_type_e type;   ///< 传输协议
} sock_conn_t;

/**
 * @brief 创建套接字
 *
 * @param type 传输类型
 * @param domain 地址类型
 * @param res 预留，标识协议
 *
 * @return int 套接字描述符，小于0异常
 */
int sock_creat_ex(sock_domain_e domain, sock_type_e type, int res);

/**
 * @brief 创建套接字
 *
 * @param domain 域类型
 * @param type 传输类型
 *
 * @return int 套接字描述符，小于0异常
 */
static inline int sock_creat(sock_domain_e domain, sock_type_e type) {
    return sock_creat_ex(domain, type, 0);
}

/**
 * @brief 创建tcp套接字
 *
 * @param domain 域类型，默认ipv4
 *
 * @return int 套接字描述符，小于0异常
 */
static inline int sock_tcp_creat(sock_domain_e domain=sock_domain_inet) {
    return sock_creat_ex(domain, sock_trans_tcp, 0);
}

/**
 * @brief 创建udp套接字
 *
 * @param domain 域类型，默认ipv4
 *
 * @return int 套接字描述符，小于0异常
 */
static inline int sock_udp_creat(sock_domain_e domain=sock_domain_inet) {
    return sock_creat_ex(domain, sock_trans_tcp, 0);
}

/**
 * @brief 绑定
 *
 * @param fd 套接字描述符
 * @param addr 地址
 *
 * @return 小于0异常，0正常
 */
int sock_bind(int fd, const sock_addr_t &addr);

/**
 * @brief 监听套接字
 *
 * @param fd 套接字描述符
 * @param blocklog
 *
 * @return 小于0异常，0正常
 */
int sock_listen(int fd, int blocklog);

/**
 * @brief accept
 *
 * @param fd 服务器套接字
 * @param addr 客户端地址
 *
 * @return 小于0异常，accept得到的客户端套接字
 */
int sock_accept(int fd, sock_addr_t &addr);

/**
 * @brief 连接到服务器
 *
 * @param fd 描述符
 * @param addr 服务器地址
 * @param ms 使用poll等待，单位ms，-1时为阻塞
 *
 * @return 
 */
int sock_connect(int fd, sock_addr_t &addr, time_t ms=-1);

/**
 * @brief 关闭套接字
 *
 * @param fd 描述符
 * @param wait 适用于tcp的延迟关闭和reset
 */
void sock_close2(int fd, time_t wait=0);

/**
 * @brief 关闭套接字
 *
 * @param fd 描述符
 */
void sock_close(int fd);

/**
 * @brief 关闭套接字
 *
 * @param fd 套接字描述符
 * @param how 方式，'r'关闭读端SHUT_RD，'w'-关闭写端SHUT_WR，'b'-关闭读写SHUT_RDWR
 */
void sock_shutdown(int fd, char how='b');

/**
 * @brief 数据发送
 * @details 只会进行一次数据写入
 *
 * @param fd 套接字描述符
 * @param data 数据
 * @param len 数据长度
 * @param w 超时时间，小于0阻塞，单位ms，使用poll来做超时
 *
 * @return 小于0异常，成功写的数据长度
 */
ssize_t sock_write(int fd, const void *data, size_t len, time_t w=-1);

/**
 * @brief 数据接收
 * @details 只会进行一次数据发送
 *
 * @param fd 套接字描述符
 * @param data 数据
 * @param len 数据长度
 * @param w 超时时间，小于0阻塞，单位ms，使用poll来做超时
 *
 * @return 小于0异常，成功读到数据长度
 */
ssize_t sock_read(int fd, void *data, size_t len, time_t w=-1);

/**
 * @brief 数据发送
 * @details 除非发送异常，否则会直到数据发送完才返回
 *
 * @param fd 套接字描述符
 * @param data 数据
 * @param len 数据长度
 * @param w 超时时间，小于0阻塞，单位ms，使用poll来做超时
 *
 * @return 小于0异常，成功写的数据长度
 */
ssize_t sock_writen(int fd, const void *data, size_t len, time_t w=-1);

/**
 * @brief 数据接收
 * @details 除非发送异常，否则会直到数据接收完才返回
 *
 * @param fd 套接字描述符
 * @param data 数据
 * @param len 数据长度
 * @param w 超时时间，小于0阻塞，单位ms，使用poll来做超时
 *
 * @return 小于0异常，成功读到数据长度
 */
ssize_t sock_readn(int fd, void *data, size_t len, time_t w=-1);

// recv/send
ssize_t sock_send(int fd, const void *data, size_t len, int flags, time_t w=-1);
ssize_t sock_recv(int fd, void *data, size_t len, int flags, time_t w=-1);

// recvfrom/sendto，用于udp
ssize_t sock_sendto(int fd, const void *data, size_t len, const sock_addr_t &addr, time_t w=-1);
ssize_t sock_recvfrom(int fd, void *data, size_t len, sock_addr_t &addr, time_t w=-1);

// 通过fd获取本地地址
int sock_get_name(int fd, sock_addr_t &addr);

// 通过fd获取对端地址
int sock_get_peer_name(int fd, sock_addr_t &addr);

// 地址重用
int sock_set_addr_reuse(int fd);

// 关闭延迟
int sock_set_nodelay(int fd, bool en = true);

// 设置keepalive
int sock_set_keepalive(int fd, bool en = true);

// 设置阻塞
int sock_set_nonblock(int fd, bool en = true);

// 设置接收超时
int sock_set_recv_timeout(int fd, time_t t);

// 设置发送超时
int sock_set_send_timeout(int fd, time_t t);

// 设置接收缓冲区
int sock_set_recv_buf_len(int fd, size_t len);

// 设置发送缓冲区
int sock_set_send_buf_len(int fd, size_t len);

} // !namespace sdk

} // !namespace aru
