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
 * @file log.hpp
 * @brief 日志接口
 * @author wotsen (astralrovers@outlook.com)
 * @version 1.0.0
 * @date 2021-04-03
 * 
 * @copyright MIT
 * 
 */
#pragma once
#include <map>
#include <string>
#include <vector>
#include <sys/time.h>
#include <glog/logging.h>
#include <glog/raw_logging.h>
#include <cstring>
#include <fstream>

#ifndef ARS_LOG_MODULE_NAME
#define ARS_LOG_MODULE_NAME "NON"
#endif

// 这里的日志接口需要指定日志实例，主要是能提供多个实例，建议实际使用时自己在对宏定义封装一次
#define ARS_LOG(severity, log_object)                                      \
    if (ars::log::filter_log_level(log_object, ARS_LOG_MODULE_NAME,       \
                                    ars::log::LogLevel::LOG_##severity)) { \
    } else                                                                  \
        LOG(severity) << "[" ARS_LOG_MODULE_NAME "] "

namespace ars {

namespace log {

typedef enum {
    LOG_INFO = 0,
    LOG_WARNING,
    LOG_ERROR,
    LOG_FATAL,
} LogLevel;

/**
 * @brief 日志等级名称
 * @details 只能使用这些等级
 *
 */
struct LogLevelName {
    constexpr static const char *level_names[] = {"INFO", "WARNING", "ERROR", "FATAL"};
};

class LogFilter;
class LogFilterImpl;

struct LogConfig {
    std::string module_name;
    LogLevel level;
};

struct LogConfigEx {};

class LogFilter {
public:
    LogFilter();
    LogFilter(const std::vector<LogConfig> &conf, const LogConfigEx *conf_ex = nullptr);
    ~LogFilter();
    void setup_log(const std::vector<LogConfig> &conf, const LogConfigEx *conf_ex = nullptr);
    bool filter(const std::string &module_name, LogLevel level) const;
    bool set_module_level(const std::string &module_name, LogLevel level);
    void remove_module(const std::string &module_name);

private:
    LogFilterImpl *impl_;
};

// 过滤日志等级
bool filter_log_level(LogFilter *log, const char *module_name, LogLevel level);

}  // namespace log

}  // namespace ars
