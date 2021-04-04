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

#include <sys/errno.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <limits.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <sys/unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>

#include "aru/sdk/net/poll.hpp"

namespace aru {

namespace sdk {

#define ARRAY_LEN(a) (sizeof(a) / sizeof(a[0]))

static int get_addr_family(int fd);

static int sock_domain_map[] = {
    AF_LOCAL,
    AF_UNIX,
    AF_INET,
    AF_ROUTE,
    PF_KEY,
    AF_INET6,
    AF_SYSTEM,
    AF_NDRV,
};

static int sock_type_map[] = {
    SOCK_STREAM,
    SOCK_DGRAM,
    SOCK_RAW,
};

static int get_addr_family(int fd) {
    struct sockaddr sin;
    socklen_t len = sizeof(sin);

    if (getsockname(fd, &sin, &len) < 0)  {
        return -1;
    }

    return sin.sa_family;
}

int sock_creat_ex(sock_domain_e domain, sock_type_e type, int res) {
    if (domain >= ARRAY_LEN(sock_domain_map) || type >= ARRAY_LEN((sock_type_map))) {
        return -1;
    }

    return socket(sock_domain_map[domain], sock_type_map[type], res);

    return 0;
}

int sock_bind(int fd, const sock_addr_t &addr) {
    struct sockaddr sin;        ///< 用于ipv4/v6的地址
    struct sockaddr_in *sin4;   ///< ipv4
    struct sockaddr_in6 *sin6;  ///< ipv6

    struct sockaddr_un un;      ///< unix地址

    struct sockaddr *real = nullptr;    ///< 实际地址
    socklen_t len = 0;                  ///< 实际地址长度
    
    // ipv4
    if (addr.type == sock_domain_inet) {
        sin4 = (struct sockaddr_in *)&sin;
        sin4->sin_family = AF_INET;
        if (addr.ip.empty()) {
            sin4->sin_addr.s_addr = htonl(INADDR_ANY);
        } else {
            sin4->sin_addr.s_addr = inet_addr(addr.ip.c_str());
        }

        if (addr.port < 0) {
            sin4->sin_port = 0;
        } else {
            sin4->sin_port = htons(addr.port);
        }

        real = &sin;
        len = sizeof(struct sockaddr_in);
    // ipv6
    } else if (addr.type == sock_domain_inet6) {
        sin6 = (struct sockaddr_in6 *)&sin;
        sin6->sin6_family = AF_INET6;
        if (addr.ip.empty()) {
            memcpy(&sin6->sin6_addr, &in6addr_any, sizeof(in6addr_any));
        } else {
            inet_pton(AF_INET6, addr.ip.c_str(), &sin6->sin6_addr);
        }

        if (addr.port < 0) {
            sin6->sin6_port = 0;
        } else {
            sin6->sin6_port = htons(addr.port);
        }

        real = &sin;
        len = sizeof(struct sockaddr_in6);
    // unix
    } else if (addr.type == sock_domain_unix) {
        un.sun_family = AF_UNIX;
        snprintf(un.sun_path, sizeof(un.sun_path), "%s", addr.ip.c_str());
        real = (struct sockaddr*)&un;
        len = sizeof(struct sockaddr_un);
    } else {
        return -1;
    }

    return bind(fd, (const struct sockaddr*)real, len);
}

int sock_listen(int fd, int blocklog) {
    return listen(fd, blocklog);
}

int sock_accept(int fd, sock_addr_t &addr) {
    struct sockaddr sin;        ///< 用于ipv4/v6的地址
    struct sockaddr_in *sin4;   ///< ipv4
    struct sockaddr_in6 *sin6;  ///< ipv6

    struct sockaddr_un un;      ///< unix地址

    struct sockaddr *real = nullptr;    ///< 实际地址
    socklen_t len = 0;                  ///< 实际地址长度

    int af = get_addr_family(fd);

    if (af == AF_INET) {
        sin4 = (struct sockaddr_in *)&sin;
        sin4->sin_family = AF_INET;
        real = &sin;
        len = sizeof(struct sockaddr_in);
    } else if (af == AF_INET6) {
        sin6 = (struct sockaddr_in6 *)&sin;
        sin6->sin6_family = AF_INET6;
        real = &sin;
        len = sizeof(struct sockaddr_in6);
    } else if (af == AF_UNIX) {
        un.sun_family = AF_UNIX;
        real = (struct sockaddr*)&un;
        len = sizeof(struct sockaddr_un);
    } else {
        return -1;
    }

    int cli = accept(fd, (struct sockaddr*)real, &len);

    if (cli <= 0) {
        return -1;
    }

    if (af != real->sa_family) {
        close(cli);
        return -1;
    }

    char ip[108] = "";

    if (af == AF_INET) {
        addr.type = sock_domain_inet;
        addr.port = ntohs(sin4->sin_port);
        inet_ntop(AF_INET, &sin4->sin_addr, ip, sizeof(sin4->sin_addr));
        addr.ip = ip;
    } else if (af == AF_INET6) {
        addr.type = sock_domain_inet6;
        addr.port = ntohs(sin6->sin6_port);
        inet_ntop(AF_INET6, &sin6->sin6_addr, ip, sizeof(sin6->sin6_addr));
        addr.ip = ip;
    } else if (af == AF_UNIX) {
        addr.type = sock_domain_unix;
        addr.ip = un.sun_path;
    } else {
        close(cli);
        return -1;
    }

    return cli;
}

int sock_connect(int fd, sock_addr_t &addr, time_t ms) {
    struct sockaddr sin;        ///< 用于ipv4/v6的地址
    struct sockaddr_in *sin4;   ///< ipv4
    struct sockaddr_in6 *sin6;  ///< ipv6

    struct sockaddr_un un;      ///< unix地址

    struct sockaddr *real = nullptr;    ///< 实际地址
    socklen_t len = 0;                  ///< 实际地址长度

    int af = get_addr_family(fd);

    if (af == AF_INET) {
        sin4 = (struct sockaddr_in *)&sin;
        sin4->sin_family = AF_INET;
        if (addr.ip.empty()) {
            sin4->sin_addr.s_addr = htonl(INADDR_ANY);
        } else {
            sin4->sin_addr.s_addr = inet_addr(addr.ip.c_str());
        }

        if (addr.port < 0) {
            return -1;
        } else {
            sin4->sin_port = htons(addr.port);
        }

        real = &sin;
        len = sizeof(struct sockaddr_in);
    } else if (af == AF_INET6) {
        sin6 = (struct sockaddr_in6 *)&sin;
        sin6->sin6_family = AF_INET6;
        if (addr.ip.empty()) {
            memcpy(&sin6->sin6_addr, &in6addr_any, sizeof(in6addr_any));
        } else {
            inet_pton(AF_INET6, addr.ip.c_str(), &sin6->sin6_addr);
        }

        if (addr.port < 0) {
            return -1;
        } else {
            sin6->sin6_port = htons(addr.port);
        }
        real = &sin;
        len = sizeof(struct sockaddr_in6);
    } else if (af == AF_UNIX) {
        un.sun_family = AF_UNIX;
        snprintf(un.sun_path, sizeof(un.sun_path), "%s", addr.ip.c_str());
        real = (struct sockaddr*)&un;
        len = sizeof(struct sockaddr_un);
    } else {
        return -1;
    }

    do {
        int r = connect(fd, real, len);

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

void sock_close(int fd) {
    close(fd);
}

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
            } else if (pr == 0) {   // 超时，继续
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
            } else if (pr == 0) {   // 超时，继续
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
    struct sockaddr sin;        ///< 用于ipv4/v6的地址
    struct sockaddr_in *sin4;   ///< ipv4
    struct sockaddr_in6 *sin6;  ///< ipv6

    struct sockaddr_un un;      ///< unix地址

    struct sockaddr *real = nullptr;    ///< 实际地址
    socklen_t alen = 0;                  ///< 实际地址长度

    int af = get_addr_family(fd);

    if (af == AF_INET) {
        sin4 = (struct sockaddr_in *)&sin;
        sin4->sin_family = AF_INET;
        if (addr.ip.empty()) {
            sin4->sin_addr.s_addr = htonl(INADDR_ANY);
        } else {
            sin4->sin_addr.s_addr = inet_addr(addr.ip.c_str());
        }

        if (addr.port < 0) {
            return -1;
        } else {
            sin4->sin_port = htons(addr.port);
        }

        real = &sin;
        alen = sizeof(struct sockaddr_in);
    } else if (af == AF_INET6) {
        sin6 = (struct sockaddr_in6 *)&sin;
        sin6->sin6_family = AF_INET6;
        if (addr.ip.empty()) {
            memcpy(&sin6->sin6_addr, &in6addr_any, sizeof(in6addr_any));
        } else {
            inet_pton(AF_INET6, addr.ip.c_str(), &sin6->sin6_addr);
        }

        if (addr.port < 0) {
            return -1;
        } else {
            sin6->sin6_port = htons(addr.port);
        }
        real = &sin;
        alen = sizeof(struct sockaddr_in6);
    } else if (af == AF_UNIX) {
        un.sun_family = AF_UNIX;
        snprintf(un.sun_path, sizeof(un.sun_path), "%s", addr.ip.c_str());
        real = (struct sockaddr*)&un;
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

    auto ret = sendto(fd, data, len, 0, real, alen);

    if (ret < 0) {
        if (errno == EAGAIN || errno == EINTR) {
            return 0;
        }
    }

    return ret;
}

ssize_t sock_recvfrom(int fd, void *data, size_t len, sock_addr_t &addr, time_t w) {
    struct sockaddr sin;        ///< 用于ipv4/v6的地址
    struct sockaddr_in *sin4;   ///< ipv4
    struct sockaddr_in6 *sin6;  ///< ipv6

    struct sockaddr_un un;      ///< unix地址

    struct sockaddr *real = nullptr;    ///< 实际地址
    socklen_t alen = 0;                  ///< 实际地址长度

    int af = get_addr_family(fd);

    if (af == AF_INET) {
        sin4 = (struct sockaddr_in *)&sin;
        sin4->sin_family = AF_INET;
        real = &sin;
        alen = sizeof(struct sockaddr_in);
    } else if (af == AF_INET6) {
        sin6 = (struct sockaddr_in6 *)&sin;
        sin6->sin6_family = AF_INET6;
        real = &sin;
        alen = sizeof(struct sockaddr_in6);
    } else if (af == AF_UNIX) {
        un.sun_family = AF_UNIX;
        real = (struct sockaddr*)&un;
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

    ret = recvfrom(fd, data, len, 0, real, &alen);

    if (ret < 0) {
        if (errno == EAGAIN || errno == EINTR) {
            return 0;
        } else {
            return -1;
        }
    }

    if (af != real->sa_family) {
        return -1;
    }

    char ip[108] = "";

    if (af == AF_INET) {
        addr.type = sock_domain_inet;
        addr.port = ntohs(sin4->sin_port);
        inet_ntop(AF_INET, &sin4->sin_addr, ip, sizeof(sin4->sin_addr));
        addr.ip = ip;
    } else if (af == AF_INET6) {
        addr.type = sock_domain_inet6;
        addr.port = ntohs(sin6->sin6_port);
        inet_ntop(AF_INET6, &sin6->sin6_addr, ip, sizeof(sin6->sin6_addr));
        addr.ip = ip;
    } else if (af == AF_UNIX) {
        addr.type = sock_domain_unix;
        addr.ip = un.sun_path;
    } else {
        return -1;
    }

    return ret;
}

// 通过fd获取本地地址
int sock_get_name(int fd, sock_addr_t &addr) {
    struct sockaddr_un un;
    struct sockaddr *sin = (struct sockaddr*)&un;
    socklen_t len = sizeof(un);

    if (getsockname(fd, sin, &len) < 0)  {
        return -1;
    }

    int af = sin->sa_family;
    struct sockaddr_in *sin4;   ///< ipv4
    struct sockaddr_in6 *sin6;  ///< ipv6

    char ip[108] = "";

    if (af == AF_INET) {
        addr.type = sock_domain_inet;
        sin4 = (struct sockaddr_in*)sin;
        addr.port = ntohs(sin4->sin_port);
        inet_ntop(AF_INET, &sin4->sin_addr, ip, sizeof(sin4->sin_addr));
        addr.ip = ip;
    } else if (af == AF_INET6) {
        addr.type = sock_domain_inet6;
        sin6 = (struct sockaddr_in6*)sin;
        addr.port = ntohs(sin6->sin6_port);
        inet_ntop(AF_INET6, &sin6->sin6_addr, ip, sizeof(sin6->sin6_addr));
        addr.ip = ip;
    } else if (af == AF_UNIX) {
        addr.type = sock_domain_unix;
        addr.ip = un.sun_path;
    } else {
        return -1;
    }

    return 0;
}

// 通过fd获取对端地址
int sock_get_peer_name(int fd, sock_addr_t &addr) {
    struct sockaddr_un un;
    struct sockaddr *sin = (struct sockaddr*)&un;
    socklen_t len = sizeof(un);

    if (getpeername(fd, sin, &len) < 0)  {
        return -1;
    }

    int af = sin->sa_family;
    struct sockaddr_in *sin4;   ///< ipv4
    struct sockaddr_in6 *sin6;  ///< ipv6

    char ip[108] = "";

    if (af == AF_INET) {
        addr.type = sock_domain_inet;
        sin4 = (struct sockaddr_in*)sin;
        addr.port = ntohs(sin4->sin_port);
        inet_ntop(AF_INET, &sin4->sin_addr, ip, sizeof(sin4->sin_addr));
        addr.ip = ip;
    } else if (af == AF_INET6) {
        addr.type = sock_domain_inet6;
        sin6 = (struct sockaddr_in6*)sin;
        addr.port = ntohs(sin6->sin6_port);
        inet_ntop(AF_INET6, &sin6->sin6_addr, ip, sizeof(sin6->sin6_addr));
        addr.ip = ip;
    } else if (af == AF_UNIX) {
        addr.type = sock_domain_unix;
        addr.ip = un.sun_path;
    } else {
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

} // !namespace sdk

} // !namespace aru
