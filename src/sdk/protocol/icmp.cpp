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
 * @file icmp.cpp
 * @brief 
 * @author wotsen (astralrovers@outlook.com)
 * @version 1.0.0
 * @date 2021-04-05
 * 
 * @copyright MIT
 * 
 */
#include "ars/sdk/protocol/icmp.hpp"
#include "ars/sdk/macros/defs.hpp"
#include "ars/sdk/err/err.hpp"
#include "ars/sdk/net/sock.hpp"
#include "ars/sdk/net/inet.hpp"
#include "ars/sdk/time/time.hpp"
#include <unistd.h>
#include <string.h>
#include <errno.h>

namespace ars {

namespace sdk {

#define PING_TIMEOUT    1000 // ms

int ping(const char* host, int cnt) {
    static uint16_t seq = 0;
    char ip[64] = {0};
#ifdef DEBUG
    uint32_t start_tick, end_tick;
#endif
    uint64_t start_hrtime, end_hrtime;
    int timeout = 0;
    int sendbytes = 64;
    char sendbuf[64];
    char recvbuf[128]; // iphdr + icmp = 84 at least
    icmp_t* icmp_req = (icmp_t*)sendbuf;
    iphdr_t* ipheader = (iphdr_t*)recvbuf;
    icmp_t* icmp_res;
    // ping stat
    int send_cnt = 0;
    int recv_cnt = 0;
    int ok_cnt = 0;
    float rtt, min_rtt, max_rtt, total_rtt;
    rtt = max_rtt = total_rtt = 0.0f;
    min_rtt = 1000000.0f;
    //min_rtt = MIN(rtt, min_rtt);
    //max_rtt = MAX(rtt, max_rtt);
    // gethostbyname -> socket -> setsockopt -> sendto -> recvfrom -> close
    sock_addr_t peeraddr;
    socklen_t addrlen = sizeof(peeraddr);
    memset(&peeraddr, 0, addrlen);
    int ret = sock_resolver(host, &peeraddr);
    if (ret != 0) return ret;
    sock_addr_ip(&peeraddr, ip, sizeof(ip));
    int sockfd = socket(peeraddr.sa.sa_family, SOCK_RAW, IPPROTO_ICMP);
    if (sockfd < 0) {
        if (errno == EPERM) {
            printe(stderr, "please use root or sudo to create a raw socket.\n");
        }
        return -errno;
    }

    timeout = PING_TIMEOUT;
    ret = sock_set_send_timeout(sockfd, timeout);
    if (ret < 0) {
        goto error;
    }
    timeout = PING_TIMEOUT;
    ret = sock_set_recv_timeout(sockfd, timeout);
    if (ret < 0) {
        goto error;
    }

    icmp_req->icmp_type = ICMP_ECHO;
    icmp_req->icmp_code = 0;
    icmp_req->icmp_id = getpid();
    for (int i = 0; i < sendbytes - (int)sizeof(icmphdr_t); ++i) {
        icmp_req->icmp_data[i] = i;
    }
#ifdef DEBUG
    start_tick = gettick();
#endif
    while (cnt-- > 0) {
        // NOTE: checksum
        icmp_req->icmp_seq = ++seq;
        icmp_req->icmp_cksum = 0;
        icmp_req->icmp_cksum = checksum((uint8_t*)icmp_req, sendbytes);
        start_hrtime = gethrtime_us();
        addrlen = sock_addr_len(&peeraddr);
        int nsend = sendto(sockfd, sendbuf, sendbytes, 0, &peeraddr.sa, addrlen);
        if (nsend < 0) {
            continue;
        }
        ++send_cnt;
        addrlen = sizeof(peeraddr);
        int nrecv = recvfrom(sockfd, recvbuf, sizeof(recvbuf), 0, &peeraddr.sa, &addrlen);
        if (nrecv < 0) {
            continue;
        }
        ++recv_cnt;
        end_hrtime = gethrtime_us();
        // check valid
        bool valid = false;
        int iphdr_len = ipheader->ihl * 4;
        int icmp_len = nrecv - iphdr_len;
        if (icmp_len == sendbytes) {
            icmp_res = (icmp_t*)(recvbuf + ipheader->ihl*4);
            if (icmp_res->icmp_type == ICMP_ECHOREPLY &&
                icmp_res->icmp_id == getpid() &&
                icmp_res->icmp_seq == seq) {
                valid = true;
            }
        }
        if (valid == false) {
            printd("recv invalid icmp packet!\n");
            continue;
        }
        rtt = (end_hrtime-start_hrtime) / 1000.0f;
        min_rtt = std::min(rtt, min_rtt);
        max_rtt = std::max(rtt, max_rtt);
        total_rtt += rtt;
        printd("%d bytes from %s: icmp_seq=%u ttl=%u time=%.1f ms\n", icmp_len, ip, seq, ipheader->ttl, rtt);
        fflush(stdout);
        ++ok_cnt;
        if (cnt > 0) sleep(1); // sleep a while, then agian
    }
#ifdef DEBUG
    end_tick = gettick();
#endif
    printd("--- %s ping statistics ---\n", host);
    printd("%d packets transmitted, %d received, %d%% packet loss, time %d ms\n",
        send_cnt, recv_cnt, (send_cnt-recv_cnt)*100/(send_cnt==0?1:send_cnt), end_tick-start_tick);
    printd("rtt min/avg/max = %.3f/%.3f/%.3f ms\n",
        min_rtt, total_rtt/(ok_cnt==0?1:ok_cnt), max_rtt);

    close(sockfd);
    return ok_cnt;
error:
    close(sockfd);
    return errno > 0 ? -errno : -1;
}

} // namespace sdk

} // namespace ars
