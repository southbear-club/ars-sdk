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
 * @file proc.hpp
 * @brief 
 * @author wotsen (astralrovers@outlook.com)
 * @version 1.0.0
 * @date 2021-04-05
 * 
 * @copyright MIT
 * 
 */
#pragma once
#include <time.h>
#include <unistd.h>

namespace aru {

namespace sdk {

/// 子进程执行函数
typedef void (*procedure_t)(void* userdata);

/**
 * @brief 子进程上下文
 * 
 */
typedef struct proc_ctx_s {
    pid_t           pid; // tid in Windows
    time_t          start_time;
    int             spawn_cnt;
    procedure_t     init;
    void*           init_userdata;
    procedure_t     proc;
    void*           proc_userdata;
    procedure_t     exit;
    void*           exit_userdata;
} proc_ctx_t;

/// 运行
static inline void proc_run(proc_ctx_t* ctx) {
    if (ctx->init) {
        ctx->init(ctx->init_userdata);
    }
    if (ctx->proc) {
        ctx->proc(ctx->proc_userdata);
    }
    if (ctx->exit) {
        ctx->exit(ctx->exit_userdata);
    }
}

/// 创建子进程
static inline int proc_spawn(proc_ctx_t* ctx) {
    ++ctx->spawn_cnt;
    ctx->start_time = time(NULL);
    pid_t pid = fork();
    if (pid < 0) {
        return -1;
    } else if (pid == 0) {
        // child process
        ctx->pid = getpid();
        proc_run(ctx);
    } else if (pid > 0) {
        // parent process
        ctx->pid = pid;
    }
    return pid;
}

} // namespace sdk

} // namespace aru
