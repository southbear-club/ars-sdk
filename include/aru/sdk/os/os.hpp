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
 * @file os.hpp
 * @brief 
 * @author wotsen (astralrovers@outlook.com)
 * @version 1.0.0
 * @date 2021-04-04
 * 
 * @copyright MIT
 * 
 */
#pragma once

namespace aru {

namespace sdk {

#if __SIZEOF_LONG__ == 8
#define ARU_64BIT 1
#elif __SIZEOF_LONG__ == 4
#define ARU_64BIT 0
#else
#error "not support"
#endif

/**
 * @brief 内存信息
 * 
 */
typedef struct {
    unsigned long total; ///< kb
    unsigned long free;  ///< kb
} meminfo_t;

// 参考 : https://github.com/ThePhD/infoware

// 获取平台架构
const char *os_get_arch(void);
// 获取平台信息
const char *os_get_platform(void);
// 获取系统名称
const char *os_get_name(void);
// 获取处理器信息
const char *os_get_processor(void);
// 获取系统版本
const char *os_get_version(void);

// 获取cpu核心数
int os_get_ncpu(void);

// 获取内存
int os_get_meminfo(meminfo_t &mem);

} // namespace sdk

} // namespace aru