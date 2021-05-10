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
 * @file consul.hpp
 * @brief 
 * @author wotsen (astralrovers@outlook.com)
 * @version 1.0.0
 * @date 2021-04-05
 * 
 * @copyright MIT
 * 
 */
#pragma once

#include <vector>
#include <string.h>

namespace ars {

namespace consul {

typedef struct consul_node_s {
    // node
    char ip[64];
    int  port;

    consul_node_s() {
        strcpy(ip, "127.0.0.1");
        port = 8500;
    }
} consul_node_t;

typedef struct consul_service_s {
    // service
    char name[64];
    char ip[64];
    int  port;

    consul_service_s() {
        memset(this, 0, sizeof(consul_service_s));
        strcpy(ip, "127.0.0.1");
    }
} consul_service_t;

typedef struct consul_health_s {
    // check
    char protocol[32]; // TCP,HTTP
    char url[256];
    char status[32]; // any,passing,warning,critical

    int interval; // ms
    int timeout;  // ms

    consul_health_s() {
        memset(this, 0, sizeof(consul_health_s));
        strcpy(protocol, "TCP");
        strcpy(status, "passing");
        interval = 10000;
        timeout = 3000;
    }
} consul_health_t;

int register_service(consul_node_t* node, consul_service_t* service, consul_health_t* health);
int deregister_service(consul_node_t* node, consul_service_t* service);
int discover_services(consul_node_t* node, const char* service_name, std::vector<consul_service_t>& services);

} // namespace consul

} // namespace ars
