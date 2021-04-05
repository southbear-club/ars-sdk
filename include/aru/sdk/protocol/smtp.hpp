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
 * @file smtp.hpp
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

#define SMTP_PORT       25
#define SMTPS_PORT      465
#define SMTP_EOB        "\r\n.\r\n"
#define SMTP_EOB_LEN    5

// smtp_command
// XX(name, string)
#define SMTP_COMMAND_MAP(XX)\
    XX(HELO,    HELO)       \
    XX(EHLO,    EHLO)       \
    XX(AUTH,    AUTH)       \
    XX(MAIL,    MAIL FROM:) \
    XX(RCPT,    RCPT TO:)   \
    XX(DATA,    DATA)       \
    XX(QUIT,    QUIT)       \

enum smtp_command {
#define XX(name, string) SMTP_##name,
    SMTP_COMMAND_MAP(XX)
#undef  XX
};

// smtp_status
// XXX(code, name, string)
#define SMTP_STATUS_MAP(XXX)                                \
    XXX(220,    READY,          Ready)                      \
    XXX(221,    BYE,            Bye)                        \
    XXX(235,    AUTH_SUCCESS,   Authentication success)     \
    XXX(250,    OK,             OK)                         \
    XXX(334,    AUTH,           Auth input)                 \
    XXX(354,    DATA,           End with <CR><LF>.<CR><LF>) \
    XXX(500,    BAD_SYNTAX,     Bad syntax)                 \
    XXX(502,    NOT_IMPLEMENTED,Command not implemented)    \
    XXX(503,    BAD_SEQUENCE,   Bad sequence of commands)   \
    XXX(504,    UNRECOGNIZED_AUTH_TYPE, Unrecognized authentication type)   \
    XXX(535,    AUTH_FAILED,    Authentication failed)      \
    XXX(553,    ERR_MAIL,       Mailbox name not allowed)   \
    XXX(554,    ERR_DATA,       Transaction failed)         \

enum smtp_status {
#define XXX(code, name, string) SMTP_STATUS_##name = code,
    SMTP_STATUS_MAP(XXX)
#undef  XXX
};

typedef struct mail_s {
    char* from;
    char* to;
    char* subject;
    char* body;
} mail_t;

const char* smtp_command_str(enum smtp_command cmd);
const char* smtp_status_str(enum smtp_status status);

// cmd param\r\n
int smtp_build_command(enum smtp_command cmd, const char* param, char* buf, int buflen);
// status_code status_message\r\n

int sendmail(const char* smtp_server,
                       const char* username,
                       const char* password,
                       mail_t* mail);
    
} // namespace sdk

} // namespace aru
