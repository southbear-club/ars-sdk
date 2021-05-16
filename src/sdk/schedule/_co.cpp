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
 * @file _co.cpp
 * @brief 
 * @author wotsen (astralrovers@outlook.com)
 * @version 1.0.0
 * @date 2021-04-21
 * 
 * @copyright MIT
 * 
 */
#include "ars/sdk/schedule/_co.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <stdio.h>

#include "ars/sdk/memory/mem.hpp"
#include "ars/sdk/ds/list.hpp"

namespace ars {
    
namespace sdk {

#define ROUND_UP(v, align) (((v) + (align) - 1) & ~((align) - 1))

typedef struct co_stack_s co_stack_t;

struct co_stack_s {
    void *stack;
    size_t stacksize;
};

struct co_s {
    list_node node;         ///< 链表节点
    co_schedule_t *sch;     ///< 调度器
    ucontext_t ctx;         ///< 上下文
    void *udata;            ///< 私有数据
    co_status_e st;         ///< 状态
    co_cb_t cb;             ///< 协程入口
    co_close_cb_t close;    ///< 结束回调
    co_stack_t stack;       ///< 协程栈
};

struct schedule_s {
    co_schedule_conf_t conf;    ///< 配置
    co_entry_t entry;           ///< 调度入口
    void *udata;                ///< 私有数据
    ucontext_t main;            ///< 主协程上下文
    size_t ns;                  ///< 协程数量
    list_node *cur;             ///< 当前正在执行的协程
    struct list_head cos;              ///< 协程链表
};

static void infunc(uint32_t low32, uint32_t hi32);
static void _del_co(co_schedule_t *sch);

co_schedule_t *co_creat(const co_schedule_conf_t *conf, co_entry_t entry, void *data) {
    if (!entry || !conf) {
        return NULL;
    }
    co_schedule_t *sch = (co_schedule_t *)ars_malloc(sizeof(co_schedule_t));

    if (!sch) {
        return NULL;
    }

    size_t page = getpagesize();
    sch->conf.limit = conf->limit;
    sch->conf.max_stack = ROUND_UP(conf->max_stack, page);
    sch->conf.min_stack = ROUND_UP(conf->min_stack, page);
    sch->conf.def_stack = ROUND_UP(conf->def_stack, page);
    if (conf->malloc) {
        sch->conf.malloc = conf->malloc;
    } else {
        sch->conf.malloc = ars_malloc;
    }

    if (conf->memalign) {
        sch->conf.memalign = conf->memalign;
    } else {
        sch->conf.memalign = ars_memalign;
    }

    if (conf->free) {
        sch->conf.free = conf->free;
    } else {
        sch->conf.free = ars_free;
    }

    sch->entry = entry;
    sch->udata = data;

    sch->ns = 0;
    sch->cur = NULL;
    list_init(&sch->cos);

    return sch;
}

int co_destroy(co_schedule_t *sch) {
    if (!sch) {
        return -1;
    }

    // 当前还有协程在执行
    if (sch->cur) {
        return -1;
    }

    list_node *pos = NULL;
    list_node *n = NULL;
    list_for_each_safe(pos, n, &sch->cos) {
        co_t *co = list_entry(pos, co_t, node);
        if (co->close) {
            co->close(co, co->udata);
        }
        list_del(pos);
        sch->conf.free(co->stack.stack);
        sch->conf.free(co);
    }

    co_free_t _free = sch->conf.free;
    _free(sch);

    return 0;
}

int co_run(co_schedule_t *sch) {
    if (!sch) {
        return -1;
    }

    return sch->entry(sch, sch->udata);
}

co_t *co_now(co_schedule_t *sch) {
    co_t *co = list_entry(sch->cur, co_t, node);

    return co;
}

size_t co_num(co_schedule_t *sch) {
    return sch->ns;
}

co_t *co_new(co_schedule_t *sch, co_cb_t cb, size_t stack, co_close_cb_t close_cb, void *udata) {
    if (!sch || !cb) {
        return NULL;
    }

    if (sch->ns >= sch->conf.limit) {
        return NULL;
    }

    co_t *new_co = (co_t*)sch->conf.malloc(sizeof(co_t));
    if (!new_co) {
        return NULL;
    }

    size_t ss = 0;

    if (stack == 0) {
        ss = sch->conf.def_stack;
    } else if (stack < sch->conf.min_stack) {
        ss = sch->conf.min_stack;
    } else if (stack > sch->conf.max_stack) {
        ss = sch->conf.max_stack;
    } else {
        return NULL;
    }

    new_co->stack.stacksize = ss;

    if (sch->conf.memalign(&new_co->stack.stack, getpagesize(), ss) < 0) {
        sch->conf.free(new_co);
        return NULL;
    }

    memset(new_co->stack.stack, 0, ss);

    new_co->sch = sch;
    list_init(&new_co->node);
    new_co->udata = udata;
    new_co->st = CO_ST_READY;
    new_co->cb = cb;
    new_co->close = close_cb;

    list_add_tail(&new_co->node, &sch->cos);
    sch->ns++;

    return new_co;
}

static void infunc(uint32_t low32, uint32_t hi32) {
    uintptr_t ptr = (uintptr_t)low32 | ((uintptr_t)hi32 << 32);
    co_schedule_t *sch = (co_schedule_t*)ptr;
    co_t *co = list_entry(sch->cur, co_t, node);
    co->cb(sch, co->udata);
    co->st = CO_ST_DEAD;
    // 不能在这里释放协程栈，因为此时还运行在协程栈上
}

static void _del_co(co_schedule_t *sch) {
    if (sch->cur) {
        co_t *co = list_entry(sch->cur, co_t, node);
        if (co->st == CO_ST_DEAD) {
            sch->ns--;
            sch->cur = NULL;
            if (co->close) {
                co->close(co, co->udata);
            }
            list_del(&co->node);
            sch->conf.free(co->stack.stack);
            sch->conf.free(co);
        }
    }
}

void co_resume(co_schedule_t *sch, co_t *co) {
    assert(sch);
    assert(co);
    assert(sch->cur == NULL);

    uintptr_t ptr = 0;
    switch (co->st) {
        case CO_ST_READY:
            getcontext(&co->ctx);
            co->ctx.uc_stack.ss_sp = co->stack.stack;
            co->ctx.uc_stack.ss_size = co->stack.stacksize;
            co->ctx.uc_link = &sch->main;
            sch->cur = &co->node;
            co->st = CO_ST_RUNNING;
            ptr = (uintptr_t)sch;
            makecontext(&co->ctx, (void(*)(void))infunc, 2, (uint32_t)ptr, (uint32_t)(ptr >> 32));
            swapcontext(&sch->main, &co->ctx);
            _del_co(sch);

            break;
        case CO_ST_SUSPEND:
            sch->cur = &co->node;
            co->st = CO_ST_RUNNING;
            swapcontext(&sch->main, &co->ctx);
            _del_co(sch);

            break;
        default:
            assert(0);
            break;
    }
}

void co_yield(co_schedule_t *sch) {
    assert(sch);
    assert(sch->cur);

    co_t *co = list_entry(sch->cur, co_t, node);

    co->st = CO_ST_SUSPEND;
    sch->cur = NULL;
    swapcontext(&co->ctx, &sch->main);
}

co_status_e co_status(co_t *co) {
    if (!co) {
        return CO_ST_DEAD;
    }

    return co->st;
}

} // namespace sdk

} // namespace ars
