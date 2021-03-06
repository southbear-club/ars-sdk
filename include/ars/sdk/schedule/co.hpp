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
 * @file co.hpp
 * @brief 
 * @author wotsen (astralrovers@outlook.com)
 * @version 1.0.0
 * @date 2021-04-21
 * 
 * @copyright MIT
 * 
 */
#pragma once

#include <stdint.h>
#include <stddef.h>
#include <functional>
#include <future>
#include <memory>

namespace ars {
    
namespace sdk {

class CoSchedule;

typedef void *(*malloc_t)(size_t size);
typedef int (*memalign_t)(void **ptr, size_t align, size_t size);
typedef void (*free_t)(void *ptr);
// 用户协程函数代理
using proxy_co_fn = std::function<void(void)>;
// 可调用对象返回类型
template <typename F, typename... Args>
using callable_ret_type = typename std::result_of<F(Args...)>::type;

CoSchedule *sch_ref(size_t def_stack, malloc_t mc, memalign_t mcalign, free_t fr);

static inline CoSchedule *sch_ref(size_t def_stack) {
    return sch_ref(def_stack, nullptr, nullptr, nullptr);
}

static inline CoSchedule *sch_ref(void) {
    ///< 默认1M的协程栈
    return sch_ref(1 * 1024 * 1024, nullptr, nullptr, nullptr);
}

/**
 * @brief 协程运行
 * 
 * @param sch 协程调度器，如果sch为空，则调度器为本线程的调度器
 */
void sch_run(CoSchedule *sch=nullptr);

/**
 * @brief 协程停止
 * 
 * @param sch 
 */
void sch_stop(CoSchedule *sch);

/**
 * @brief 让出执行权
 * 
 */
void yield(void);

/**
 * @brief 内部接口，添加协程任务
 * 
 * @param stack 栈大小
 * @param fn 协程函数
 */
void __add_co(size_t stack, proxy_co_fn fn);

/// 添加协程任务，支持不定参数
template <typename F, typename... Args>
void new_co(F &&f, Args &&... args) {
	auto call = std::bind(std::forward<F>(f), std::forward<Args>(args)...);

    __add_co(0, [call]() { call(); });
}

/// 添加协程任务，支持不定参数，协程栈大小
template <typename F, typename... Args>
void new_co(size_t stack, F &&f, Args &&... args) {
	auto call = std::bind(std::forward<F>(f), std::forward<Args>(args)...);

    __add_co(stack, [call]() { (*call)(); });
}

} // namespace sdk

} // namespace ars
