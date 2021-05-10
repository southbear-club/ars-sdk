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
 * @file dns.hpp
 * @brief 
 * @author wotsen (astralrovers@outlook.com)
 * @version 1.0.0
 * @date 2021-04-05
 * 
 * @copyright MIT
 * 
 */
#pragma once
#include <stdint.h>
#include "../macros/defs.hpp"

namespace ars {
    
namespace sdk {

#define ARS_DNS_PORT        53

#define ARS_DNS_QUERY       0
#define ARS_DNS_RESPONSE    1

#define ARS_DNS_TYPE_A      1   // ipv4
#define ARS_DNS_TYPE_NS     2
#define ARS_DNS_TYPE_CNAME  5
#define ARS_DNS_TYPE_SOA    6
#define ARS_DNS_TYPE_WKS    11
#define ARS_DNS_TYPE_PTR    12
#define ARS_DNS_TYPE_HINFO  13
#define ARS_DNS_TYPE_MX     15
#define ARS_DNS_TYPE_AAAA   28  // ipv6
#define ARS_DNS_TYPE_AXFR   252
#define ARS_DNS_TYPE_ANY    255

#define ARS_DNS_CLASS_IN    1

#define ARS_DNS_NAME_MAXLEN 256

// sizeof(dnshdr_t) = 12
typedef struct dnshdr_s {
    uint16_t    transaction_id;
    // flags
#if ARS_BYTE_ORDER == ARS_LITTLE_ENDIAN
    uint8_t     rd:1;
    uint8_t     tc:1;
    uint8_t     aa:1;
    uint8_t     opcode:4;
    uint8_t     qr:1;

    uint8_t     rcode:4;
    uint8_t     cd:1;
    uint8_t     ad:1;
    uint8_t     res:1;
    uint8_t     ra:1;
#elif ARS_BYTE_ORDER == ARS_BIG_ENDIAN
    uint8_t    qr:1;   // DNS_QUERY or DNS_RESPONSE
    uint8_t    opcode:4;
    uint8_t    aa:1;   // authoritative
    uint8_t    tc:1;   // truncated
    uint8_t    rd:1;   // recursion desired

    uint8_t    ra:1;   // recursion available
    uint8_t    res:1;  // reserved
    uint8_t    ad:1;   // authenticated data
    uint8_t    cd:1;   // checking disable
    uint8_t    rcode:4;
#else
#error "BYTE_ORDER undefined!"
#endif
    uint16_t    nquestion;
    uint16_t    nanswer;
    uint16_t    nauthority;
    uint16_t    naddtional;
} dnshdr_t;

typedef struct dns_rr_s {
    char        name[ARS_DNS_NAME_MAXLEN]; // original domain, such as www.example.com
    uint16_t    rtype;
    uint16_t    rclass;
    uint32_t    ttl;
    uint16_t    datalen;
    char*       data;
} dns_rr_t;

typedef struct dns_s {
    dnshdr_t        hdr;
    dns_rr_t*       questions;
    dns_rr_t*       answers;
    dns_rr_t*       authorities;
    dns_rr_t*       addtionals;
} dns_t;

// www.example.com => 3www7example3com
int dns_name_encode(const char* domain, char* buf);
// 3www7example3com => www.example.com
int dns_name_decode(const char* buf, char* domain);

int dns_rr_pack(dns_rr_t* rr, char* buf, int len);
int dns_rr_unpack(char* buf, int len, dns_rr_t* rr, int is_question);

int dns_pack(dns_t* dns, char* buf, int len);
int dns_unpack(char* buf, int len, dns_t* dns);
// NOTE: free dns->rrs
void dns_free(dns_t* dns);

// dns_pack -> sendto -> recvfrom -> dns_unpack
int dns_query(dns_t* query, dns_t* response, const char* nameserver = "127.0.1,1");

// domain -> dns_t query; -> dns_query -> dns_t response; -> addrs
int nslookup(const char* domain, uint32_t* addrs, int naddr, const char* nameserver = "127.0.1.1");

} // namespace sdk

} // namespace ars
