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
 * @file ftp.cpp
 * @brief 
 * @author wotsen (astralrovers@outlook.com)
 * @version 1.0.0
 * @date 2021-04-05
 * 
 * @copyright MIT
 * 
 */
#include "ars/sdk/protocol/ftp.hpp"
#include "ars/sdk/err/err.hpp"
#include "ars/sdk/net/sock.hpp"
#include <unistd.h>

namespace ars {
    
namespace sdk {

const char* ftp_command_str(enum ftp_command cmd) {
    switch (cmd) {
#define X(name) case FTP_##name: return #name;
    FTP_COMMAND_MAP(X)
#undef  X
    default: return "<unknown>";
    }
}

const char* ftp_status_str(enum ftp_status status) {
    switch (status) {
#define XXX(code, name, string) case FTP_STATUS_##name: return #string;
    FTP_STATUS_MAP(XXX)
#undef  XXX
    default: return "<unknown>";
    }
}

int ftp_connect(ftp_handle_t* hftp, const char* host, int port) {
    int sockfd = sock_tcp_creat();
    if (sockfd < 0) {
        return sockfd;
    }

    sock_addr_t raddr;
    sock_set_ipport(&raddr, host, port);

    int cret = sock_connect(sockfd, raddr, 5000);
    if (cret < 0) {
        close(sockfd);
        return cret;
    }

    sock_set_send_timeout(sockfd, 5000);
    sock_set_recv_timeout(sockfd, 5000);
    hftp->sockfd = sockfd;
    int ret = 0;
    int status_code = 0;
    memset(hftp->recvbuf, 0, FTP_RECV_BUFSIZE);
    int nrecv = recv(sockfd, hftp->recvbuf, FTP_RECV_BUFSIZE, 0);
    if (nrecv <= 0) {
        ret = ARS_ERR_RECV;
        goto error;
    }
    status_code = atoi(hftp->recvbuf);
    if (status_code != FTP_STATUS_READY) {
        ret = status_code;
        goto error;
    }
    return 0;

error:
    close(sockfd);
    return ret;
}

int ftp_login(ftp_handle_t* hftp, const char* username, const char* password) {
    int status_code = ftp_exec(hftp, "USER", username);
    status_code = ftp_exec(hftp, "PASS", password);
    return status_code == FTP_STATUS_LOGIN_OK ? 0 : status_code;
}

int ftp_quit(ftp_handle_t* hftp) {
    ftp_exec(hftp, "QUIT", NULL);
    close(hftp->sockfd);
    return 0;
}

int ftp_exec(ftp_handle_t* hftp, const char* cmd, const char* param) {
    char buf[1024];
    int len = 0;
    if (param && *param) {
        len = snprintf(buf, sizeof(buf), "%s %s\r\n", cmd, param);
    }
    else {
        len = snprintf(buf, sizeof(buf), "%s\r\n", cmd);
    }
    int nsend, nrecv;
    int ret = 0;
    nsend = send(hftp->sockfd, buf, len, 0);
    if (nsend != len) {
        ret = ARS_ERR_SEND;
        goto error;
    }
    //printf("> %s", buf);
    memset(hftp->recvbuf, 0, FTP_RECV_BUFSIZE);
    nrecv = recv(hftp->sockfd, hftp->recvbuf, FTP_RECV_BUFSIZE, 0);
    if (nrecv <= 0) {
        ret = ARS_ERR_RECV;
        goto error;
    }
    //printf("< %s", hftp->recvbuf);
    return atoi(hftp->recvbuf);
error:
    close(hftp->sockfd);
    return ret;
}

static int ftp_parse_pasv(const char* resp, char* host, int* port) {
    // 227 Entering Passive Mode (127,0,0,1,4,51)
    const char* str = strchr(resp, '(');
    if (str == NULL) {
        return ARS_ERR_RESPONSE;
    }
    int arr[6];
    sscanf(str, "(%d,%d,%d,%d,%d,%d)",
            &arr[0], &arr[1], &arr[2], &arr[3], &arr[4], &arr[5]);
    sprintf(host, "%d.%d.%d.%d", arr[0], arr[1], arr[2], arr[3]);
    *port = arr[4] << 8 | arr[5];
    return 0;
}

int ftp_download_with_cb(ftp_handle_t* hftp, const char* filepath, ftp_download_cb cb) {
    int status_code = ftp_exec(hftp, "PASV", NULL);
    if (status_code != FTP_STATUS_PASV) {
        return status_code;
    }
    char host[64];
    int port = 0;
    int ret = ftp_parse_pasv(hftp->recvbuf, host, &port);
    if (ret != 0) {
        return ret;
    }
    //ftp_exec(hftp, "RETR", filepath);
    char request[1024];
    int len = snprintf(request, sizeof(request), "RETR %s\r\n", filepath);
    int nsend = send(hftp->sockfd, request, len, 0);
    if (nsend != len) {
        close(hftp->sockfd);
        return ARS_ERR_SEND;
    }
    //printf("> %s", request);
    int sockfd = sock_tcp_creat();
    if (sockfd < 0) {
        return sockfd;
    }

    sock_addr_t raddr;
    sock_set_ipport(&raddr, host, port);

    int cret = sock_connect(sockfd, raddr, 5000);
    if (cret < 0) {
        close(sockfd);
        return cret;
    }

    int nrecv = recv(hftp->sockfd, hftp->recvbuf, FTP_RECV_BUFSIZE, 0);
    if (nrecv <= 0) {
        close(hftp->sockfd);
        return ARS_ERR_RECV;
    }
    //printf("< %s", hftp->recvbuf);
    {
        // you can create thread to recv data
        char recvbuf[1024];
        int ntotal = 0;
        while (1) {
            nrecv = recv(sockfd, recvbuf, sizeof(recvbuf), 0);
            if (cb) {
                cb(hftp, recvbuf, nrecv);
            }
            if (nrecv <= 0) break;
            ntotal += nrecv;
        }
    }
    close(sockfd);
    nrecv = recv(hftp->sockfd, hftp->recvbuf, FTP_RECV_BUFSIZE, 0);
    if (nrecv <= 0) {
        close(hftp->sockfd);
        return ARS_ERR_RECV;
    }
    //printf("< %s", hftp->recvbuf);
    status_code = atoi(hftp->recvbuf);
    return status_code == FTP_STATUS_TRANSFER_COMPLETE ? 0 : status_code;
}

// local => remote
int ftp_upload(ftp_handle_t* hftp, const char* local_filepath, const char* remote_filepath) {
    int status_code = ftp_exec(hftp, "PASV", NULL);
    if (status_code != FTP_STATUS_PASV) {
        return status_code;
    }
    char host[64];
    int port = 0;
    int ret = ftp_parse_pasv(hftp->recvbuf, host, &port);
    if (ret != 0) {
        return ret;
    }
    //ftp_exec(hftp, "STOR", remote_filepath);
    char request[1024];
    int len = snprintf(request, sizeof(request), "STOR %s\r\n", remote_filepath);
    int nsend = send(hftp->sockfd, request, len, 0);
    if (nsend != len) {
        close(hftp->sockfd);
        return ARS_ERR_SEND;
    }
    //printf("> %s", request);

    int sockfd = sock_tcp_creat();
    if (sockfd < 0) {
        return sockfd;
    }

    sock_addr_t raddr;
    sock_set_ipport(&raddr, host, port);

    int cret = sock_connect(sockfd, raddr, 5000);
    if (cret < 0) {
        close(sockfd);
        return cret;
    }

    int nrecv = recv(hftp->sockfd, hftp->recvbuf, FTP_RECV_BUFSIZE, 0);
    if (nrecv <= 0) {
        close(hftp->sockfd);
        return ARS_ERR_RECV;
    }
    //printf("< %s", hftp->recvbuf);
    {
        // you can create thread to send data
        FILE* fp = fopen(local_filepath, "rb");
        if (fp == NULL) {
            close(sockfd);
            return ARS_ERR_OPEN_FILE;
        }
        char sendbuf[1024];
        int nread, nsend;
        int ntotal = 0;
        while (1) {
            nread = fread(sendbuf, 1, sizeof(sendbuf), fp);
            if (nread == 0) break;
            nsend = send(sockfd, sendbuf, nread, 0);
            if (nsend != nread) break;
            ntotal += nsend;
        }
        fclose(fp);
    }
    close(sockfd);
    nrecv = recv(hftp->sockfd, hftp->recvbuf, FTP_RECV_BUFSIZE, 0);
    if (nrecv <= 0) {
        close(hftp->sockfd);
        return ARS_ERR_RECV;
    }
    //printf("< %s", hftp->recvbuf);
    status_code = atoi(hftp->recvbuf);
    return status_code == FTP_STATUS_TRANSFER_COMPLETE ? 0 : status_code;
}

static int s_ftp_download_cb(ftp_handle_t* hftp, char* buf, int len) {
    FILE* fp = (FILE*)hftp->userdata;
    if (fp == NULL) return -1;
    if (len <= 0) {
        fclose(fp);
        hftp->userdata = NULL;
        return 0;
    }
    return fwrite(buf, 1, len, fp);
}

// remote => local
int ftp_download(ftp_handle_t* hftp, const char* remote_filepath, const char* local_filepath) {
    FILE* fp = fopen(local_filepath, "wb");
    if (fp == NULL) {
        return ARS_ERR_OPEN_FILE;
    }
    hftp->userdata = (void*)fp;
    int ret = ftp_download_with_cb(hftp, remote_filepath, s_ftp_download_cb);
    // ensure fclose
    if (hftp->userdata != NULL) {
        fclose(fp);
        hftp->userdata = NULL;
    }
    return ret;
}
    
} // namespace sdk

} // namespace ars
