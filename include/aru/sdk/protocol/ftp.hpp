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
 * @file ftp.hpp
 * @brief 
 * @author wotsen (astralrovers@outlook.com)
 * @version 1.0.0
 * @date 2021-04-05
 * 
 * @copyright MIT
 * 
 */
#pragma once

namespace aru {
    
namespace sdk {

#define FTP_COMMAND_PORT    21
#define FTP_DATA_PORT       20

// ftp_command
// X(name)
#define FTP_COMMAND_MAP(X) \
    X(HELP)     \
    X(USER)     \
    X(PASS)     \
    X(PWD)      \
    X(CWD)      \
    X(CDUP)     \
    X(MKD)      \
    X(RMD)      \
    X(STAT)     \
    X(SIZE)     \
    X(DELE)     \
    X(RNFR)     \
    X(RNTO)     \
    X(PORT)     \
    X(PASV)     \
    X(LIST)     \
    X(NLST)     \
    X(APPE)     \
    X(RETR)     \
    X(STOR)     \
    X(QUIT)     \

enum ftp_command {
#define X(name) FTP_##name,
    FTP_COMMAND_MAP(X)
#undef  X
};

// ftp_status
// XXX(code, name, string)
#define FTP_STATUS_MAP(XXX) \
    XXX(220,    READY,          Ready)  \
    XXX(221,    BYE,            Bye)    \
    XXX(226,    TRANSFER_COMPLETE,  Transfer complete)  \
    XXX(227,    PASV,           Entering Passive Mode)  \
    XXX(331,    PASS,           Password required)      \
    XXX(230,    LOGIN_OK,       Login OK)   \
    XXX(250,    OK,             OK)         \
    XXX(500,    BAD_SYNTAX,     Bad syntax)         \
    XXX(530,    NOT_LOGIN,      Not login)  \

enum ftp_status {
#define XXX(code, name, string) FTP_STATUS_##name = code,
    FTP_STATUS_MAP(XXX)
#undef  XXX
};

// more friendly macros
#define FTP_MKDIR       FTP_MKD
#define FTP_RMDIR       FTP_RMD
#define FTP_APPEND      FTP_APPE
#define FTP_REMOVE      FTP_DELE
#define FTP_DOWNLOAD    FTP_RETR
#define FTP_UPLOAD      FTP_STOR

#define FTP_RECV_BUFSIZE    8192

typedef struct ftp_handle_s {
    int     sockfd;
    char    recvbuf[FTP_RECV_BUFSIZE];
    void*   userdata;
} ftp_handle_t;

const char* ftp_command_str(enum ftp_command cmd);
const char* ftp_status_str(enum ftp_status status);

int ftp_connect(ftp_handle_t* hftp, const char* host, int port);
int ftp_login(ftp_handle_t* hftp, const char* username, const char* password);
int ftp_quit(ftp_handle_t* hftp);

int ftp_exec(ftp_handle_t* hftp, const char* cmd, const char* param);

// local => remote
int ftp_upload(ftp_handle_t* hftp, const char* local_filepath, const char* remote_filepath);
// remote => local
int ftp_download(ftp_handle_t* hftp, const char* remote_filepath, const char* local_filepath);

typedef int (*ftp_download_cb)(ftp_handle_t* hftp, char* buf, int len);
int ftp_download_with_cb(ftp_handle_t* hftp, const char* filepath, ftp_download_cb cb);
    
} // namespace sdk

} // namespace aru
