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
 * @file sock.cpp
 * @brief
 * @author wotsen (astralrovers@outlook.com)
 * @version 1.0.0
 * @date 2021-04-04
 *
 * @copyright MIT
 *
 */
#include "aru/sdk/net/sock.hpp"

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/unistd.h>
#include <unistd.h>

#include "aru/sdk/macros/defs.hpp"
#include "aru/sdk/net/poll.hpp"

namespace aru {

namespace sdk {

#define ARRAY_LEN(a) (sizeof(a) / sizeof(a[0]))

int sock_family(int fd) {
    struct sockaddr sin;
    socklen_t len = sizeof(sin);

    if (getsockname(fd, &sin, &len) < 0) {
        return -1;
    }

    return sin.sa_family;
}

int sock_bind(int fd, const char *ip, int port) {
    sock_addr_t addr;

    socklen_t len = 0;  ///< 实际地址长度

    int f = sock_family(fd);

    addr.sa.sa_family = f;

    // ipv4
    if (f == AF_INET) {
        if (sock_set_ipport(&addr, ip, port) < 0) {
            return -1;
        }
        len = sizeof(struct sockaddr_in);
        // ipv6
    } else if (f == AF_INET6) {
        if (sock_set_ipport(&addr, ip, port) < 0) {
            return -1;
        }
        len = sizeof(struct sockaddr_in6);
        // unix
    } else if (f == AF_UNIX) {
        sock_set_path(&addr, ip);
        len = sizeof(struct sockaddr_un);
    } else {
        return -1;
    }

    return bind(fd, (const struct sockaddr *)&addr, len);
}

int sock_accept(int fd, sock_addr_t &addr) {
    struct sockaddr_in *sin4;   ///< ipv4
    struct sockaddr_in6 *sin6;  ///< ipv6
    struct sockaddr_un *un;     ///< unix地址

    socklen_t len = 0;  ///< 实际地址长度

    int af = sock_family(fd);

    if (af == AF_INET) {
        sin4 = (struct sockaddr_in *)&addr;
        sin4->sin_family = AF_INET;
        len = sizeof(struct sockaddr_in);
    } else if (af == AF_INET6) {
        sin6 = (struct sockaddr_in6 *)&addr;
        sin6->sin6_family = AF_INET6;
        len = sizeof(struct sockaddr_in6);
    } else if (af == AF_UNIX) {
        un = (struct sockaddr_un *)&addr;
        un->sun_family = AF_UNIX;
        len = sizeof(struct sockaddr_un);
    } else {
        return -1;
    }

    int cli = accept(fd, (struct sockaddr *)&addr, &len);

    if (cli <= 0) {
        return -1;
    }

    return cli;
}

int sock_connect(int fd, sock_addr_t &addr, time_t ms) {
    struct sockaddr_in *sin4;   ///< ipv4
    struct sockaddr_in6 *sin6;  ///< ipv6
    struct sockaddr_un *un;     ///< unix地址

    socklen_t len = 0;  ///< 实际地址长度

    int af = sock_family(fd);

    if (af == AF_INET) {
        sin4 = (struct sockaddr_in *)&addr;
        sin4->sin_family = AF_INET;
        len = sizeof(struct sockaddr_in);
    } else if (af == AF_INET6) {
        sin6 = (struct sockaddr_in6 *)&addr;
        sin6->sin6_family = AF_INET6;
        len = sizeof(struct sockaddr_in6);
    } else if (af == AF_UNIX) {
        un = (struct sockaddr_un *)&addr;
        un->sun_family = AF_UNIX;
        len = sizeof(struct sockaddr_un);
    } else {
        return -1;
    }

    do {
        int r = connect(fd, (const struct sockaddr *)&addr, len);

        if (r == 0) {
            return 0;
        }

        if (errno == EINPROGRESS) {
            if (ms >= 0) {
                if (sdk::poll_wait(fd, 'r', ms) < 0) {
                    return -1;
                }
            }

            int err = -1;
            socklen_t elen = sizeof(err);

            r = getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &elen);

            if (r != 0) return -1;
            if (err == 0) return 0;

            errno = err;

            return -1;
        } else if (errno != EINTR) {
            return -1;
        }
    } while (true);

    return -1;
}

void sock_close(int fd) { close(fd); }

void sock_close2(int fd, time_t wait) {
    struct linger lin;

    lin.l_onoff = 1;
    lin.l_linger = wait;

    setsockopt(fd, SOL_SOCKET, SO_LINGER, &lin, sizeof(lin));

    close(fd);
}

void sock_shutdown(int fd, char how) {
    if (how == 'r') {
        shutdown(fd, SHUT_RD);
    } else if (how == 'w') {
        shutdown(fd, SHUT_WR);
    } else {
        shutdown(fd, SHUT_RDWR);
    }
}

ssize_t sock_write(int fd, const void *data, size_t len, time_t w) {
    if (w >= 0) {
        int r = sdk::poll_wait(fd, 'w', w);

        if (r < 0) {
            return -1;
        } else if (r == 0) {
            return 0;
        } else {
        }
    }

    auto ret = write(fd, data, len);

    if (ret < 0) {
        if (errno == EAGAIN || errno == EINTR) {
            return 0;
        } else {
        }
    }

    return ret;
}

ssize_t sock_read(int fd, void *data, size_t len, time_t w) {
    if (w >= 0) {
        int r = sdk::poll_wait(fd, 'r', w);

        if (r < 0) {
            return -1;
        } else if (r == 0) {
            return 0;
        } else {
        }
    }

    auto ret = read(fd, data, len);
    if (ret < 0) {
        if (errno == EAGAIN || errno == EINTR) {
            return 0;
        } else {
        }
    }

    return ret;
}

ssize_t sock_writen(int fd, const void *data, size_t len, time_t w) {
    ssize_t r = 0;
    ssize_t total = 0;
    ssize_t left = len;

    while (left) {
        if (w < 0) {
            r = write(fd, data, len - total);
        } else {
            int pr = sdk::poll_wait(fd, 'w', w);

            if (pr < 0) {
                return -1;
            } else if (pr == 0) {  // 超时，继续
                continue;
            } else {
                r = write(fd, data, len - total);
            }
        }

        if (r < 0) {
            if (errno == EAGAIN || errno == EINTR) {
                continue;
            } else {
                return total;
            }
        }

        if (r == 0) {
            continue;
        }

        total += r;
        left -= r;
    }

    return total;
}

ssize_t sock_readn(int fd, void *data, size_t len, time_t w) {
    ssize_t r = 0;
    ssize_t total = 0;
    ssize_t left = len;

    while (left) {
        if (w < 0) {
            r = read(fd, data, len - total);
        } else {
            int pr = sdk::poll_wait(fd, 'r', w);

            if (pr < 0) {
                return -1;
            } else if (pr == 0) {  // 超时，继续
                continue;
            } else {
                r = read(fd, data, len - total);
            }
        }

        if (r < 0) {
            if (errno == EAGAIN || errno == EINTR) {
                continue;
            } else {
                return total;
            }
        }

        if (r == 0) {
            continue;
        }

        total += r;
        left -= r;
    }

    return total;
}

ssize_t sock_send(int fd, const void *data, size_t len, int flags, time_t w) {
    if (w >= 0) {
        int r = sdk::poll_wait(fd, 'w', w);

        if (r < 0) {
            return -1;
        } else if (r == 0) {
            return 0;
        } else {
        }
    }

    auto ret = send(fd, data, len, flags);
    if (ret < 0) {
        if (errno == EAGAIN || errno == EINTR) {
            return 0;
        }
    }

    return ret;
}

ssize_t sock_recv(int fd, void *data, size_t len, int flags, time_t w) {
    if (w >= 0) {
        int r = sdk::poll_wait(fd, 'r', w);

        if (r < 0) {
            return -1;
        } else if (r == 0) {
            return 0;
        }
    }

    auto ret = recv(fd, data, len, flags);
    if (ret < 0) {
        if (errno == EAGAIN || errno == EINTR) {
            return 0;
        }
    }

    return ret;
}

ssize_t sock_sendto(int fd, const void *data, size_t len, const sock_addr_t &addr, time_t w) {
    struct sockaddr_in *sin4;   ///< ipv4
    struct sockaddr_in6 *sin6;  ///< ipv6
    struct sockaddr_un *un;     ///< unix地址

    socklen_t alen = 0;  ///< 实际地址长度

    int af = sock_family(fd);

    if (af == AF_INET) {
        sin4 = (struct sockaddr_in *)&addr;
        sin4->sin_family = AF_INET;
        alen = sizeof(struct sockaddr_in);
    } else if (af == AF_INET6) {
        sin6 = (struct sockaddr_in6 *)&addr;
        sin6->sin6_family = AF_INET6;
        alen = sizeof(struct sockaddr_in6);
    } else if (af == AF_UNIX) {
        un = (struct sockaddr_un *)&addr;
        un->sun_family = AF_UNIX;
        alen = sizeof(struct sockaddr_un);
    } else {
        return -1;
    }

    if (w >= 0) {
        int r = sdk::poll_wait(fd, 'w', w);

        if (r < 0) {
            return -1;
        } else if (r == 0) {
            return 0;
        } else {
        }
    }

    auto ret = sendto(fd, data, len, 0, (const struct sockaddr *)&addr, alen);

    if (ret < 0) {
        if (errno == EAGAIN || errno == EINTR) {
            return 0;
        }
    }

    return ret;
}

ssize_t sock_recvfrom(int fd, void *data, size_t len, sock_addr_t &addr, time_t w) {
    struct sockaddr_in *sin4;   ///< ipv4
    struct sockaddr_in6 *sin6;  ///< ipv6
    struct sockaddr_un *un;     ///< unix地址

    socklen_t alen = 0;  ///< 实际地址长度

    int af = sock_family(fd);

    if (af == AF_INET) {
        sin4 = (struct sockaddr_in *)&addr;
        sin4->sin_family = AF_INET;
        alen = sizeof(struct sockaddr_in);
    } else if (af == AF_INET6) {
        sin6 = (struct sockaddr_in6 *)&addr;
        sin6->sin6_family = AF_INET6;
        alen = sizeof(struct sockaddr_in6);
    } else if (af == AF_UNIX) {
        un = (struct sockaddr_un *)&addr;
        un->sun_family = AF_UNIX;
        alen = sizeof(struct sockaddr_un);
    } else {
        return -1;
    }

    ssize_t ret = -1;

    if (w >= 0) {
        int r = sdk::poll_wait(fd, 'r', w);

        if (r < 0) {
            return -1;
        } else if (r == 0) {
            return 0;
        } else {
        }
    }

    ret = recvfrom(fd, data, len, 0, (struct sockaddr *)&addr, &alen);

    if (ret < 0) {
        if (errno == EAGAIN || errno == EINTR) {
            return 0;
        } else {
            return -1;
        }
    }

    return ret;
}

// 通过fd获取本地地址
int sock_get_name(int fd, sock_addr_t &addr) {
    struct sockaddr *sin = (struct sockaddr *)&addr;
    int af = sock_family(fd);
    socklen_t len = 0;

    if (af == AF_INET) {
        len = sizeof(struct sockaddr_in);
        sin->sa_family = af;
    } else if (af == AF_INET6) {
        len = sizeof(struct sockaddr_in6);
        sin->sa_family = af;
    } else if (af == AF_UNIX) {
        len = sizeof(struct sockaddr_un);
        sin->sa_family = af;
    } else {
        return -1;
    }

    if (getsockname(fd, sin, &len) < 0) {
        return -1;
    }

    return 0;
}

// 通过fd获取对端地址
int sock_get_peer_name(int fd, sock_addr_t &addr) {
    struct sockaddr *sin = (struct sockaddr *)&addr;
    int af = sock_family(fd);
    socklen_t len = 0;

    if (af == AF_INET) {
        len = sizeof(struct sockaddr_in);
        sin->sa_family = af;
    } else if (af == AF_INET6) {
        len = sizeof(struct sockaddr_in6);
        sin->sa_family = af;
    } else if (af == AF_UNIX) {
        len = sizeof(struct sockaddr_un);
        sin->sa_family = af;
    } else {
        return -1;
    }

    if (getpeername(fd, sin, &len) < 0) {
        return -1;
    }

    return 0;
}

// 地址重用
int sock_set_addr_reuse(int fd) {
    int v = 1;
    return setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &v, sizeof(v));
}

// 关闭延迟
int sock_set_nodelay(int fd, bool en) {
    int v = en ? 1 : 0;
    return setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &v, sizeof(v));
}

// 设置keepalive
int sock_set_keepalive(int fd, bool en) {
    int v = en ? 1 : 0;
    return setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &v, sizeof(v));
}

// 设置阻塞
int sock_set_nonblock(int fd, bool en) {
    int ret = fcntl(fd, F_GETFL);

    if (ret < 0) {
        return -1;
    }

    if (en) {
        ret |= O_NONBLOCK;
    } else {
        ret &= ~O_NONBLOCK;
    }

    return fcntl(fd, F_SETFL, ret);
}

// 设置接收超时
int sock_set_recv_timeout(int fd, time_t t) {
    struct timeval tm;
    tm.tv_sec = t / 1000;
    tm.tv_usec = t % 1000 * 1000;

    return setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tm, sizeof(tm));
}

// 设置发送超时
int sock_set_send_timeout(int fd, time_t t) {
    struct timeval tm;
    tm.tv_sec = t / 1000;
    tm.tv_usec = t % 1000 * 1000;

    return setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tm, sizeof(tm));
}

// 设置接收缓冲区
int sock_set_recv_buf_len(int fd, size_t len) {
    return setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &len, sizeof(len));
}

// 设置发送缓冲区
int sock_set_send_buf_len(int fd, size_t len) {
    return setsockopt(fd, SOL_SOCKET, SO_SNDBUF, &len, sizeof(len));
}

int sock_pair(int family, int type, int protocol, int sv[2]) {
    if (family == AF_UNIX) {
        return ::socketpair(family, type, protocol, sv);
    }
    if (family != AF_INET || type != SOCK_STREAM) {
        return -1;
    }
    int listenfd, connfd, acceptfd;
    listenfd = connfd = acceptfd = ARU_INVALID_SOCKET;
    struct sockaddr_in localaddr;
    socklen_t addrlen = sizeof(localaddr);
    memset(&localaddr, 0, addrlen);
    localaddr.sin_family = AF_INET;
    localaddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    localaddr.sin_port = 0;
    // listener
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0) {
        goto error;
    }
    if (bind(listenfd, (struct sockaddr *)&localaddr, addrlen) < 0) {
        goto error;
    }
    if (listen(listenfd, 1) < 0) {
        goto error;
    }
    if (getsockname(listenfd, (struct sockaddr *)&localaddr, &addrlen) < 0) {
        goto error;
    }
    // connector
    connfd = socket(AF_INET, SOCK_STREAM, 0);
    if (connfd < 0) {
        goto error;
    }
    if (connect(connfd, (struct sockaddr *)&localaddr, addrlen) < 0) {
        goto error;
    }
    // acceptor
    acceptfd = accept(listenfd, (struct sockaddr *)&localaddr, &addrlen);
    if (acceptfd < 0) {
        goto error;
    }

    close(listenfd);
    sv[0] = connfd;
    sv[1] = acceptfd;
    return 0;
error:
    if (listenfd != ARU_INVALID_SOCKET) {
        close(listenfd);
    }
    if (connfd != ARU_INVALID_SOCKET) {
        close(connfd);
    }
    if (acceptfd != ARU_INVALID_SOCKET) {
        close(acceptfd);
    }
    return -1;
}

socklen_t sock_addr_len(sock_addr_t *addr) {
    if (addr->sa.sa_family == AF_INET) {
        return sizeof(struct sockaddr_in);
    } else if (addr->sa.sa_family == AF_INET6) {
        return sizeof(struct sockaddr_in6);
    } else if (addr->sa.sa_family == AF_UNIX) {
        return sizeof(struct sockaddr_un);
    }
    return sizeof(sock_addr_t);
}

const char *sock_addr_str(sock_addr_t *addr, char *buf, int len) {
    char ip[ARU_SOCKADDR_STRLEN] = {0};
    uint16_t port = 0;
    if (addr->sa.sa_family == AF_INET) {
        inet_ntop(AF_INET, &addr->sin.sin_addr, ip, len);
        port = htons(addr->sin.sin_port);
        snprintf(buf, len, "%s:%d", ip, port);
    } else if (addr->sa.sa_family == AF_INET6) {
        inet_ntop(AF_INET6, &addr->sin6.sin6_addr, ip, len);
        port = htons(addr->sin6.sin6_port);
        snprintf(buf, len, "[%s]:%d", ip, port);
    } else if (addr->sa.sa_family == AF_UNIX) {
        snprintf(buf, len, "%s", addr->sun.sun_path);
    }
    return buf;
}

int sock_resolver(const char *host, sock_addr_t *addr) {
    if (inet_pton(AF_INET, host, &addr->sin.sin_addr) == 1) {
        addr->sa.sa_family = AF_INET;  // host is ipv4, so easy ;)
        return 0;
    }

    if (inet_pton(AF_INET6, host, &addr->sin6.sin6_addr) == 1) {
        addr->sa.sa_family = AF_INET6;  // host is ipv6
        return 0;
    }
    struct addrinfo *ais = NULL;

    int ret = getaddrinfo(host, NULL, NULL, &ais);
    if (ret != 0 || ais == NULL || ais->ai_addrlen == 0 || ais->ai_addr == NULL) {
        printd("unknown host: %s err:%d:%s\n", host, ret, gai_strerror(ret));
        return ret;
    }
    memcpy(addr, ais->ai_addr, ais->ai_addrlen);
    freeaddrinfo(ais);
    return 0;
}

const char *sock_addr_ip(sock_addr_t *addr, char *ip, int len) {
    if (addr->sa.sa_family == AF_INET) {
        return inet_ntop(AF_INET, &addr->sin.sin_addr, ip, len);
    } else if (addr->sa.sa_family == AF_INET6) {
        return inet_ntop(AF_INET6, &addr->sin6.sin6_addr, ip, len);
    }
    return ip;
}

uint16_t sock_addr_port(sock_addr_t *addr) {
    uint16_t port = 0;
    if (addr->sa.sa_family == AF_INET) {
        port = htons(addr->sin.sin_port);
    } else if (addr->sa.sa_family == AF_INET6) {
        port = htons(addr->sin6.sin6_port);
    }
    return port;
}

void sock_set_family(sock_addr_t* addr, int family) {
    addr->sa.sa_family = family;
}

int sock_set_ip(sock_addr_t *addr, const char *host) {
    if (!host || *host == '\0') {
        if (addr->sa.sa_family == AF_INET) {
            addr->sin.sin_addr.s_addr = htonl(INADDR_ANY);
        } else if (addr->sa.sa_family == AF_INET6) {
            memcpy(&addr->sin6.sin6_addr, &in6addr_any, sizeof(in6addr_any));
        } else {
            return -1;
        }
        return 0;
    }
    return sock_resolver(host, addr);
}

void sock_set_port(sock_addr_t *addr, int port) {
    if (addr->sa.sa_family == AF_INET) {
        addr->sin.sin_port = ntohs(port);
    } else if (addr->sa.sa_family == AF_INET6) {
        addr->sin6.sin6_port = ntohs(port);
    }
}

int sock_set_ipport(sock_addr_t *addr, const char *host, int port) {
    int ret = sock_set_ip(addr, host);
    if (ret != 0) return ret;
    sock_set_port(addr, port);
    return 0;
}

}  // namespace sdk

}  // namespace aru
