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
 * @brief 
 * @author  ()
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

namespace aru {
    
namespace sdk {

#ifndef PTHREAD_STACK_MIN
#define PTHREAD_STACK_MIN 16384
#endif

#define THREAD_ONCE_RUN_INIT PTHREAD_ONCE_INIT

#define ARU_THREAD_NAME_LEN 16

typedef pthread_t thread_t;
typedef pthread_key_t thread_key_t;
typedef pthread_attr_t thread_attr_t;
typedef pthread_once_t thread_once_t;

typedef void *(*thread_cb_t)(void*);
typedef void (*thread_key_del_t)(void*);

int thread_bind_cpu(pthread_t tid, int cpu);
void thread_set_default_stacksize(uint64_t size);

// ------------------thread key start---------------------
static inline int thread_key_create(thread_key_t *key, thread_key_del_t *fn) {
    return ::pthread_key_create(key, (void(*)(void*))fn);
}

static inline int thread_key_delete(thread_key_t key) {
    return ::pthread_key_delete(key);
}

static inline void *thread_key_get_specific(thread_key_t key) {
    return ::pthread_getspecific(key);
}

static inline int thread_key_set_specific(thread_key_t key, const void *value) {
    return ::pthread_setspecific(key, value);
}

// ------------------thread key end---------------------

#if !defined(__APPLE__) && defined(__linux__)
static inline int thread_get_cpuclockid(thread_t tid, clockid_t *id) {
    return ::pthread_getcpuclockid(tid, id);
}
#endif

static inline int thread_attr_init(thread_attr_t *attr) {
    return ::pthread_attr_init(attr);
}

#if !defined(__APPLE__) && defined(__linux__)
static inline int thread_get_attr(thread_t tid, thread_attr_t *attr) {
    return ::pthread_getattr_np(tid, attr);
}
#endif

static inline int thread_attr_destroy(thread_attr_t *attr) {
    return ::pthread_attr_destroy(attr);
}

#if !defined(__APPLE__) && defined(__linux__)
static inline int thread_attr_get_def(thread_attr_t *attr) {
    return ::pthread_getattr_default_np(attr);
}

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

// PTHREAD_CREATE_DETACHED
// PTHREAD_CREATE_JOINABLE
static inline int thread_attr_set_detachstate(thread_attr_t *attr, int detachstate) {
    return ::pthread_attr_setdetachstate(attr, detachstate);
}

static inline int thread_attr_get_detachstate(thread_attr_t *attr, int *detatchstate) {
    return ::pthread_attr_getdetachstate(attr, detatchstate);
}

static inline int thread_attr_set_guardsize(thread_attr_t *attr, size_t guardsize) {
    return ::pthread_attr_setguardsize(attr, guardsize);
}

static inline int thread_attr_get_guardsize(thread_attr_t *attr, size_t *guardsize) {
    return ::pthread_attr_getguardsize(attr, guardsize);
}

// PTHREAD_INHERIT_SCHED
// PTHREAD_EXPLICIT_SCHED
static inline int thread_attr_set_inherit_sched(thread_attr_t *attr, int inherit_sched) {
    return ::pthread_attr_setinheritsched(attr, inherit_sched);
}

static inline int thread_attr_get_inheri_sched(thread_attr_t *attr, int *inherit_sched) {
    return ::pthread_attr_getinheritsched(attr, inherit_sched);
}

static inline int thread_attr_set_schedparam(thread_attr_t *attr, const struct sched_param *param) {
    return ::pthread_attr_setschedparam(attr, param);
}

static inline int thread_attr_get_schedparam(thread_attr_t *attr, struct sched_param *param) {
    return ::pthread_attr_getschedparam(attr, param);
}

// SCHED_FIDO
// SCHED_RR
// SCHED_OTHER
static inline int thread_attr_set_schedpolicy(thread_attr_t *attr, int policy) {
    return ::pthread_attr_setschedpolicy(attr, policy);
}

static inline int thread_attr_get_schedpolicy(thread_attr_t *attr, int *policy) {
    return ::pthread_attr_getschedpolicy(attr, policy);
}

// PTHREAD_SCOPE_SYSTEM
// PTHREAD_SCOPE_PROCESS
static inline int thread_attr_set_scope(thread_attr_t *attr, int scope) {
    return ::pthread_attr_setscope(attr, scope);
}

static inline int thread_attr_get_scope(thread_attr_t *attr, int *scope) {
    return ::pthread_attr_getscope(attr, scope);
}

static inline int thread_attr_setstack(thread_attr_t *attr, void *stackaddr, size_t stacksize) {
    return ::pthread_attr_setstack(attr, stackaddr, stacksize);
}

static inline int thread_attr_get_statck(thread_attr_t *attr, void **stackaddr, size_t *stacksize) {
    return ::pthread_attr_getstack(attr, stackaddr, stacksize);
}

static inline int thread_attr_set_stacksize(thread_attr_t *attr, size_t stacksize) {
    return ::pthread_attr_setstacksize(attr, stacksize);
}

static inline int thread_attr_get_stacksize(thread_attr_t *attr, size_t *stacksize) {
    return ::pthread_attr_getstacksize(attr, stacksize);
}

static inline int thread_set_concurrency(int new_level) {
    return ::pthread_setconcurrency(new_level);
}

static inline int thread_get_concurrency(void) {
    return ::pthread_getconcurrency();
}

static inline int thread_set_name(thread_t tid, const char *name) {
#ifdef __APPLE__
    if (tid) {}
    return ::pthread_setname_np(name);
#else
    return ::pthread_setname_np(tid, name);
#endif
}

static inline int thread_get_name(thread_t tid, char *name, size_t len) {
    return ::pthread_getname_np(tid, name, len);
}

static inline int thread_affork(void (*pre)(void), void (*parent)(void), void (*child)(void)) {
    return ::pthread_atfork(pre, parent, child);
}

static inline int thread_create(thread_t *tid, thread_cb_t fun, const thread_attr_t *attr, void *arg) {
    return ::pthread_create(tid, attr, (void*(*)(void*))fun, arg);
}

static inline int thread_yield(void) {
    return ::sched_yield();
}

static inline void thread_exit(void *ret) {
    return ::pthread_exit(ret);
}

static inline int thread_cancel(thread_t tid) {
    return ::pthread_cancel(tid);
}

// PTHREAD_CANCEL_ENABLE
// PTHREAD_CANCEL_DISABLE
static inline int thread_set_cancel_state(int state, int *oldstate) {
    return ::pthread_setcancelstate(state, oldstate);
}

// PTHREAD_CANCEL_DEFERRED
// PTHREAD_CANCEL_ASYNCHRONOUS
static inline int thread_set_cancel_type(int type, int *oldtype) {
    return ::pthread_setcanceltype(type, oldtype);
}

static inline void thread_cancle_test(void) {
    ::pthread_testcancel();
}

static inline int thread_send_signal(thread_t tid, int sig) {
    return ::pthread_kill(tid, sig);
}

static inline bool thread_alive(thread_t tid) {
    return ::pthread_kill(tid, 0);
}

static inline int thread_sigmask(int how, const sigset_t *set, sigset_t *oldset) {
    return ::pthread_sigmask(how, set, oldset);
}

#if !defined(__APPLE__) && defined(__linux__)
static inline int thread_sigqueue(thread_t tid, int sig, const union sigval value) {
    return ::pthread_sigqueue(tid, sig, value);
}
#endif

static inline int thread_detach(thread_t tid) {
    return ::pthread_detach(tid);
}

static inline int thread_join(thread_t tid, void **ret) {
    return ::pthread_join(tid, ret);
}

static inline thread_t gettid(void) {
    return pthread_self();
}

#define aru_thread_cleanup_push(rountine, arg) pthread_cleanup_push(rountine, arg)

#define aru_thread_cleanup_pop(execute) pthread_cleanup_pop(execute)

static inline int thread_once(thread_once_t *once_cntl, void (*init_routine)(void)) {
    return ::pthread_once(once_cntl, init_routine);
}

static inline int thread_cmp(thread_t t1, thread_t t2) {
    return ::pthread_equal(t1, t2);
}

} // namespace sdk

} // namespace aru
