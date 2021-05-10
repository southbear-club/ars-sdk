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
 * @file dl.hpp
 * @brief 加载动态库
 * @author wotsen (astralrovers@outlook.com)
 * @version 1.0.0
 * @date 2021-04-10
 * 
 * @copyright MIT
 * 
 */
#pragma once
#include <dlfcn.h>

namespace ars {
    
namespace sdk {

/// 打开动态库
static inline void *dlopen(const char *path, int mode) {
    return ::dlopen(path, mode);
}

/// 关闭动态库
static inline int dlclose(void *handle) {
    return ::dlclose(handle);
}

/// 动态库内符号查找
static inline void *dlsym(void *handle, const char *symbol) {
    return ::dlsym(handle, symbol);
}

/// 操作过程中的错误
static inline const char *dlerror(void) {
    return ::dlerror();
}

// static inline bool dlopen_precheck(const char *path) {
//     return ::dlopen_preflight(path);
// }

} // namespace sdk

} // namespace ars
