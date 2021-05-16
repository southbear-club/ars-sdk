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
 * @file thread_util.hpp
 * @brief 线程基本接口
 * @author wotsen (astralrovers@outlook.com)
 * @version 1.0.0
 * @date 2021-04-07
 * 
 * @copyright MIT
 * 
 */
#pragma once

#include <pthread.h>
#include <sched.h>
#include <stdint.h>
#include <time.h>
#include <signal.h>

namespace ars {
    
namespace sdk {

/// 最小栈
#ifndef PTHREAD_STACK_MIN
#define PTHREAD_STACK_MIN 16384
#endif

/// 执行一次初始化值
#define THREAD_ONCE_RUN_INIT PTHREAD_ONCE_INIT

/// 线程名长度
#define ARS_THREAD_NAME_LEN 16

/// 线程类型
typedef pthread_t thread_t;
/// 线程key
typedef pthread_key_t thread_key_t;
/// 线程属性
typedef pthread_attr_t thread_attr_t;
/// 执行一次
typedef pthread_once_t thread_once_t;

/// 线程任务
typedef void *(*thread_cb_t)(void*);
/// key删除回调
typedef void (*thread_key_del_t)(void*);

/**
 * @brief 绑定cpu
 * 
 * @param tid 线程号
 * @param cpu cpu号
 * @return int 0成功，小于0异常
 */
int thread_bind_cpu(pthread_t tid, int cpu);

/**
 * @brief 设置全局默认线程栈大小
 * 
 * @param size 栈大小
 */
void thread_set_default_stacksize(uint64_t size);

// ------------------thread key start---------------------

/**
 * @brief 创建key
 * 
 * @param key key指针
 * @param fn 删除回调
 * @return int 0成功，小于0异常
 */
static inline int thread_key_create(thread_key_t *key, thread_key_del_t *fn) {
    return ::pthread_key_create(key, (void(*)(void*))fn);
}

/**
 * @brief 删除key
 * 
 * @param key key指针
 * @return int 0成功，小于0异常
 */
static inline int thread_key_delete(thread_key_t key) {
    return ::pthread_key_delete(key);
}

/**
 * @brief 获取key值
 * 
 * @param key key指针
 * @return void* 值
 */
static inline void *thread_key_get_specific(thread_key_t key) {
    return ::pthread_getspecific(key);
}

/**
 * @brief 设置key值
 * 
 * @param key key指针
 * @param value 值
 * @return int 0成功，小于0异常
 */
static inline int thread_key_set_specific(thread_key_t key, const void *value) {
    return ::pthread_setspecific(key, value);
}

// ------------------thread key end---------------------

#if !defined(__APPLE__) && defined(__linux__)
/// 获取线程cpu时间周期
static inline int thread_get_cpuclockid(thread_t tid, clockid_t *id) {
    return ::pthread_getcpuclockid(tid, id);
}
#endif

/**
 * @brief 线程属性初始化
 * 
 * @param attr 属性
 * @return int 0成功，小于0异常
 */
static inline int thread_attr_init(thread_attr_t *attr) {
    return ::pthread_attr_init(attr);
}

#if !defined(__APPLE__) && defined(__linux__)
/// 获取线程属性
static inline int thread_get_attr(thread_t tid, thread_attr_t *attr) {
    return ::pthread_getattr_np(tid, attr);
}
#endif

/**
 * @brief 删除线程属性
 * 
 * @param attr 属性
 * @return int 0成功，小于0异常
 */
static inline int thread_attr_destroy(thread_attr_t *attr) {
    return ::pthread_attr_destroy(attr);
}

#if !defined(__APPLE__) && defined(__linux__)
/// 获取默认线程属性
static inline int thread_attr_get_def(thread_attr_t *attr) {
    return ::pthread_getattr_default_np(attr);
}

/// 设置默认线程属性
static inline int thread_attr_set_def(thread_attr_t *attr) {
    return ::pthread_setattr_default_np(attr);
}

static inline int thread_attr_set_affinity(thread_attr_t *attr, size_t cpuset_size, const cpu_set_t *cpuset) {
    return ::pthread_attr_setaffinity_np(attr, cpuset_size, cpuset);
}

static inline int thread_attr_get_affinity(thread_attr_t *attr, size_t cpuset_size, cpu_set_t *cpuset) {
    return ::pthread_attr_getaffinity_np(attr, cpuset_size, cpuset);
}

#endif

/**
 * @brief 设置线程分离状态
 * 
 * @param attr 属性
 * @param detachstate PTHREAD_CREATE_DETACHED --> 分离，PTHREAD_CREATE_JOINABLE --> 等待
 * @return int 0成功，小于0异常
 */
static inline int thread_attr_set_detachstate(thread_attr_t *attr, int detachstate) {
    return ::pthread_attr_setdetachstate(attr, detachstate);
}

/**
 * @brief 获取线程分离状态
 * 
 * @param attr 属性
 * @param detachstate PTHREAD_CREATE_DETACHED --> 分离，PTHREAD_CREATE_JOINABLE --> 等待
 * @return int 0成功，小于0异常
 */
static inline int thread_attr_get_detachstate(thread_attr_t *attr, int *detatchstate) {
    return ::pthread_attr_getdetachstate(attr, detatchstate);
}

/**
 * @brief 设置线程保护栈大小，越界了不至于导致程序崩溃
 * 
 * @param attr 属性
 * @param guardsize 保护栈大小
 * @return int 0成功，小于0异常
 */
static inline int thread_attr_set_guardsize(thread_attr_t *attr, size_t guardsize) {
    return ::pthread_attr_setguardsize(attr, guardsize);
}

/// 获取线程保护栈大小
static inline int thread_attr_get_guardsize(thread_attr_t *attr, size_t *guardsize) {
    return ::pthread_attr_getguardsize(attr, guardsize);
}

/**
 * @brief 设置继承的调度策略
 * 
 * @param attr 属性
 * @param inherit_sched 调度策略 PTHREAD_INHERIT_SCHED，PTHREAD_EXPLICIT_SCHED
 * @return int 0成功，小于0异常
 */
static inline int thread_attr_set_inherit_sched(thread_attr_t *attr, int inherit_sched) {
    return ::pthread_attr_setinheritsched(attr, inherit_sched);
}

/// 设置继承调度策略
static inline int thread_attr_get_inherit_sched(thread_attr_t *attr, int *inherit_sched) {
    return ::pthread_attr_getinheritsched(attr, inherit_sched);
}

/**
 * @brief 设置线程优先级
 * 
 * @param attr 属性
 * @param param 优先级 50 - 90
 * @return int 0成功，小于0异常
 */
static inline int thread_attr_set_schedparam(thread_attr_t *attr, const struct sched_param *param) {
    return ::pthread_attr_setschedparam(attr, param);
}

/// 获取优先级
static inline int thread_attr_get_schedparam(thread_attr_t *attr, struct sched_param *param) {
    return ::pthread_attr_getschedparam(attr, param);
}

/**
 * @brief 设置线程调度策略
 * 
 * @param attr 属性
 * @param policy 调度属性 SCHED_FIDO,SCHED_RR,SCHED_OTHER
 * @return int 0成功，小于0异常
 */
static inline int thread_attr_set_schedpolicy(thread_attr_t *attr, int policy) {
    return ::pthread_attr_setschedpolicy(attr, policy);
}

/// 获取调度策略
static inline int thread_attr_get_schedpolicy(thread_attr_t *attr, int *policy) {
    return ::pthread_attr_getschedpolicy(attr, policy);
}

/**
 * @brief 设置线程属性作用域
 * 
 * @param attr 属性
 * @param scope 作用域 PTHREAD_SCOPE_SYSTEM,PTHREAD_SCOPE_PROCESS
 * @return int 0成功，小于0异常
 */
static inline int thread_attr_set_scope(thread_attr_t *attr, int scope) {
    return ::pthread_attr_setscope(attr, scope);
}

/// 获取属性作用域
static inline int thread_attr_get_scope(thread_attr_t *attr, int *scope) {
    return ::pthread_attr_getscope(attr, scope);
}

static inline int thread_attr_setstack(thread_attr_t *attr, void *stackaddr, size_t stacksize) {
    return ::pthread_attr_setstack(attr, stackaddr, stacksize);
}

static inline int thread_attr_get_statck(thread_attr_t *attr, void **stackaddr, size_t *stacksize) {
    return ::pthread_attr_getstack(attr, stackaddr, stacksize);
}

/**
 * @brief 设置线程栈大小
 * 
 * @param attr 属性
 * @param stacksize 栈大小
 * @return int 0成功，小于0异常
 */
static inline int thread_attr_set_stacksize(thread_attr_t *attr, size_t stacksize) {
    return ::pthread_attr_setstacksize(attr, stacksize);
}

/// 获取线程栈大小
static inline int thread_attr_get_stacksize(thread_attr_t *attr, size_t *stacksize) {
    return ::pthread_attr_getstacksize(attr, stacksize);
}

/// 设置进程并发线程数
static inline int thread_set_concurrency(int new_level) {
    return ::pthread_setconcurrency(new_level);
}

/// 获取进程并发线程数
static inline int thread_get_concurrency(void) {
    return ::pthread_getconcurrency();
}

/// 设置线程名
static inline int thread_set_name(thread_t tid, const char *name) {
#ifdef __APPLE__
    if (tid) {}
    if (name.empty()) {}
    // return ::pthread_setname_np(name);
    return 0;
#else
    return ::pthread_setname_np(tid, name);
#endif
}

/// 获取线程名
static inline int thread_get_name(thread_t tid, char *name, size_t len) {
    return ::pthread_getname_np(tid, name, len);
}

static inline int thread_affork(void (*pre)(void), void (*parent)(void), void (*child)(void)) {
    return ::pthread_atfork(pre, parent, child);
}

/**
 * @brief 创建线程
 * 
 * @param tid[out] 线程id
 * @param fun 执行函数
 * @param attr 属性
 * @param arg 参数
 * @return int 0成功，小于0异常
 */
static inline int thread_create(thread_t *tid, thread_cb_t fun, const thread_attr_t *attr, void *arg) {
    return ::pthread_create(tid, attr, (void*(*)(void*))fun, arg);
}

/**
 * @brief 让出执行权
 * 
 * @return int 0成功，小于0异常
 */
static inline int thread_yield(void) {
    return ::sched_yield();
}

/**
 * @brief 线程退出
 * 
 * @param ret 输出结果
 */
static inline void thread_exit(void *ret) {
    return ::pthread_exit(ret);
}

/**
 * @brief 线程取消
 * 
 * @param tid 线程id
 * @return int 0成功，小于0异常
 */
static inline int thread_cancel(thread_t tid) {
    return ::pthread_cancel(tid);
}

/**
 * @brief 设置线程取消使能
 * 
 * @param state 使能 PTHREAD_CANCEL_ENABLE,PTHREAD_CANCEL_DISABLE
 * @param oldstate 旧使能
 * @return int 0成功，小于0异常
 */
static inline int thread_set_cancel_state(int state, int *oldstate) {
    return ::pthread_setcancelstate(state, oldstate);
}

/**
 * @brief 设置取消类型
 * 
 * @param type 类型 PTHREAD_CANCEL_DEFERRED --> 同步取消，下个锚点取消掉,PTHREAD_CANCEL_ASYNCHRONOUS --> 异步取消，立即退出
 * @param oldtype 旧类型
 * @return int 0成功，小于0异常
 */
static inline int thread_set_cancel_type(int type, int *oldtype) {
    return ::pthread_setcanceltype(type, oldtype);
}

/// 取消测试，如果你的线程内没有sleep这种锚点，那么你需要调用该接口来设置锚点
static inline void thread_cancle_test(void) {
    ::pthread_testcancel();
}

/**
 * @brief 向线程发送信号
 * 
 * @param tid 线程id
 * @param sig 信号
 * @return int 0成功，小于0异常
 */
static inline int thread_send_signal(thread_t tid, int sig) {
    return ::pthread_kill(tid, sig);
}

/// 测试线程是否存活
static inline bool thread_alive(thread_t tid) {
    return ::pthread_kill(tid, 0);
}

/// 信号屏蔽
static inline int thread_sigmask(int how, const sigset_t *set, sigset_t *oldset) {
    return ::pthread_sigmask(how, set, oldset);
}

#if !defined(__APPLE__) && defined(__linux__)
static inline int thread_sigqueue(thread_t tid, int sig, const union sigval value) {
    return ::pthread_sigqueue(tid, sig, value);
}
#endif

/// 线程分离
static inline int thread_detach(thread_t tid) {
    return ::pthread_detach(tid);
}

/// 线程join
static inline int thread_join(thread_t tid, void **ret) {
    return ::pthread_join(tid, ret);
}

/// 获取线程id
static inline thread_t gettid(void) {
    return pthread_self();
}

/// 线程保护入栈
#define ars_thread_cleanup_push(rountine, arg) pthread_cleanup_push(rountine, arg)

/// 线程保护出栈
#define ars_thread_cleanup_pop(execute) pthread_cleanup_pop(execute)

/**
 * @brief 仅执行一次
 * 
 * @param once_cntl once变量
 * @param init_routine 执行函数
 * @return int 0成功，小于0异常
 */
static inline int thread_once(thread_once_t *once_cntl, void (*init_routine)(void)) {
    return ::pthread_once(once_cntl, init_routine);
}

/**
 * @brief 线程比较
 * 
 * @param t1 线程1
 * @param t2 线程2
 * @return int 同一个线程时非0，其他情况为0
 */
static inline int thread_cmp(thread_t t1, thread_t t2) {
    return ::pthread_equal(t1, t2);
}

} // namespace sdk

} // namespace ars
