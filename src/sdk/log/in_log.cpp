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
 * @file in_log.cpp
 * @brief 
 * @author wotsen (astralrovers@outlook.com)
 * @version 1.0.0
 * @date 2021-04-03
 * 
 * @copyright MIT
 * 
 */
#include "in_log.hpp"
#include "ars/sdk/ars.hpp"

namespace ars {

namespace log {

// 内部日志
static LogFilter log_filter;

LogFilter *get_log_filter(void) { return &log_filter; }

}  // namespace log

}  // namespace ars

namespace ars {

namespace lib {

// 一键设置日志等级
void set_log_level(ars::log::LogLevel level) {
#define XX(m, name) name,
    const char *logs[] = {ARS_MODULE_TABLE(XX)};
#undef XX

    for (auto item : logs) {
        ars::log::log_filter.set_module_level(item, level);
    }
}

// 设置单个模块日志等级
void set_log_level(const std::string &module_name, ars::log::LogLevel level) {
    ars::log::log_filter.set_module_level(module_name, level);
}

}  // namespace lib

}  // namespace ars
