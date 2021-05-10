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
 * @file match.cpp
 * @brief 
 * @author wotsen (astralrovers@outlook.com)
 * @version 1.0.0
 * @date 2021-04-04
 * 
 * @copyright MIT
 * 
 */
#include "ars/sdk/str/match.hpp"
#include <regex>

namespace ars {

namespace sdk {

static bool _match(const std::string &str, const std::string &re_str) {
    std::cmatch cm;
    std::regex re(re_str.c_str());

    return std::regex_match(str.c_str(), cm, re);
}

bool email_match(const std::string &email) {
    return _match(
        email,
        "^([A-Za-z0-9_\\-\\.])+\\@([A-Za-z0-9_\\-\\.])+\\.([A-Za-z]{2,4})$");
}

bool id_card_match(const std::string &number) {
    return _match(number, "(^\\d{15}$)|(^\\d{17}([0-9]|X)$)");
}

bool zh_cellphone_number_match(const std::string &number) {
    return _match(number, "^1[3456789]\\d{9}$");
}

bool ipv4_match(const std::string &ip) {
    return _match(ip + ".",
                  "^((\\d|[1-9]\\d|1\\d\\d|2([0-4]\\d|5[0-5]))\\.){4}$");
}

bool ipv6_match(const std::string &ip) {
    return _match(ip + ":", "^(([\\da-fA-F]{1,4}):){8}$");
}

}  // namespace sdk

}  // namespace ars
