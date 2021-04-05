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
 * @file ifconfig.hpp
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

namespace aru {

namespace sdk {

typedef struct ifconfig_s {
    char name[128];
    char ip[16];
    char mask[16];
    char broadcast[16];
    char mac[20];
} ifconfig_t;

/*
 *  @test
    std::vector<ifconfig_t> ifcs;
    ifconfig(ifcs);
    for (auto& item : ifcs) {
        printf("%s\nip: %s\nmask: %s\nbroadcast: %s\nmac: %s\n\n",
                item.name,
                item.ip,
                item.mask,
                item.broadcast,
                item.mac);
    }
 */
int ifconfig(std::vector<ifconfig_t>& ifcs);

} // namespace sdk

} // namespace aru
