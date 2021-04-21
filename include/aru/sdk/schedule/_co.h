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
 * @file _co.h
 * @brief 
 * @author wotsen (astralrovers@outlook.com)
 * @version 1.0.0
 * @date 2021-04-21
 * 
 * @copyright MIT
 * 
 */
#pragma once

#include <stddef.h>
#include <stdint.h>
#if __APPLE__
#define _XOPEN_SOURCE
	#include <ucontext.h>
#else 
	#include <ucontext.h>
#endif 

namespace aru {
    
namespace sdk {

///< 调度器
typedef struct schedule_s co_schedule_t;
///< 协程
typedef struct co_s co_t;
///< 调度器入口函数
typedef int (*co_entry_t)(co_schedule_t *sch, void *ud);
///< 协程回调接口
typedef void (*co_cb_t)(co_schedule_t *, void *ud);
///< 协程结束后的回调接口
typedef void (*co_close_cb_t)(co_t *co, void *ud);

typedef void *(*co_malloc_t)(size_t size);
typedef int (*co_memalign_t)(void **ptr, size_t align, size_t size);
typedef void (*co_free_t)(void *ptr);

/**
 * @brief 协程状态
 * 
 */
typedef enum {
    CO_ST_DEAD,         ///< 终止
    CO_ST_READY,        ///< 就绪
    CO_ST_RUNNING,      ///< 运行中
    CO_ST_SUSPEND,      ///< 挂起
} co_status_e;

/**
 * @brief 协程调度配置
 * 
 */
typedef struct schedule_conf_s {
    size_t limit;           ///< 协程数量限制
    size_t def_stack;       ///< 默认运行栈
    size_t max_stack;       ///< 协程最大运行栈
    size_t min_stack;       ///< 最小运行栈
    co_malloc_t malloc;     ///< 一般内存分配，默认malloc
    co_memalign_t memalign; ///< 协程栈分配，默认posix_memalign
    co_free_t free;         ///< 内存释放，默认free
} co_schedule_conf_t;

/**
 * @brief 创建协程调度器
 * 
 * @param conf 配置
 * @param entry 调度器入口函数
 * @param data 私有数据
 * @return co_schedule_t* 调度器句柄
 */
co_schedule_t *co_creat(const co_schedule_conf_t *conf, co_entry_t entry, void *data);

/**
 * @brief 销毁协程调度器
 * 
 * @param sch 调度器
 * @return int 0成功，小于0异常，有协程在运行时会失败
 */
int co_destroy(co_schedule_t *sch);

/**
 * @brief 运行调度器
 * 
 * @param sch 调度器
 * @return int 0成功，小于0异常
 */
int co_run(co_schedule_t *sch);

/**
 * @brief 获取当前正在运行的协程
 * 
 * @param sch 调度器
 * @return co_t* 协程句柄
 */
co_t *co_now(co_schedule_t *sch);

/**
 * @brief 当前协程数量
 * 
 * @param sch 调度器
 * @return size_t 
 */
size_t co_num(co_schedule_t *sch);

/**
 * @brief 创建协程
 * @details 协程结束后内存会自动回收
 * 
 * @param sch 调度器
 * @param cb 协程入口程序
 * @param stack 协程栈，为0时使用默认栈大小
 * @param close 协程结束后的回调接口
 * @param udata 私有数据
 * @return co_t* 
 */
co_t *co_new(co_schedule_t *sch, co_cb_t cb, size_t stack, co_close_cb_t close_cb, void *udata);

/**
 * @brief 协程恢复运行
 * 
 * @param sch 调度器
 * @param co 协程
 */
void co_resume(co_schedule_t *sch, co_t *co);

/**
 * @brief 协程暂停，让出执行权
 * 
 * @param sch 调度器
 */
void co_yield(co_schedule_t *sch);

/**
 * @brief 协程状态
 * 
 * @param co 协程
 * @return co_status_e 当前状态
 */
co_status_e co_status(co_t *co);

} // namespace sdk

} // namespace aru
