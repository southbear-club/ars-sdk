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
 * @file co.cpp
 * @brief 
 * @author wotsen (astralrovers@outlook.com)
 * @version 1.0.0
 * @date 2021-04-21
 * 
 * @copyright MIT
 * 
 */
#include <assert.h>
#include <limits.h>
#include <stdlib.h>
#include <vector>
#include <condition_variable>
#include <mutex>
#include <stdio.h>
#include "aru/sdk/schedule/co.hpp"
#include "aru/sdk/schedule/_co.h"
#include "aru/sdk/ds/list.hpp"

namespace aru {
    
namespace sdk {

typedef struct {
    list_node node;
    co_t *co;
} co_id_t;

struct CoContext {
    proxy_co_fn cb;
    void *udata;
    co_id_t *id;
};

class CoSchedule {
public:
    CoSchedule() : sch_(nullptr), stop_(false), stoped_(true), malloc_(malloc), memalign_(posix_memalign), free_(free), cur_(nullptr) {
        list_init(&cos_);
    }
    ~CoSchedule() {
        co_destroy(sch_);
        sch_ = nullptr;
    }
    void init(size_t def_stack, malloc_t mc, memalign_t mcalign, free_t fr) {
        if (!sch_) {
            co_schedule_conf_t conf = {
                ULONG_MAX,
                def_stack,
                4 * 1024 * 1024,
                4096,
                mc,
                mcalign,
                fr,
            };
            if (mc) {
                malloc_ = mc;
            } else {
                malloc_ = ::malloc;
            }
            if (mcalign) {
                memalign_ = mcalign;
            } else {
                memalign_ = ::posix_memalign;
            }
            if (fr) {
                free_ = fr;
            } else {
                free_ = ::free;
            }
            sch_ = co_creat(&conf, entry, this);
            assert(sch_);
        }
    }

    void run(void) {
        {
            std::unique_lock<std::mutex> lck(mtx_);
            if (!stoped_) {
                return;
            }

            stoped_ = false;
            stop_ = false;
        }

        co_run(sch_);
    }

    void stop(void) {
        {
            std::unique_lock<std::mutex> lck(mtx_);
            if (stoped_) {
                return;
            }

            stop_ = true;

            cond_.wait(lck, [this] { return stoped_; });
        }
    }

    void add(size_t stack, proxy_co_fn fn) {
        CoContext *ctx = (CoContext*)malloc_(sizeof(CoContext));
        if (!ctx) {
            return;
        }

        co_id_t *id = (co_id_t*)malloc_(sizeof(co_id_t));
        if (!id) {
            free_(ctx);
            return;
        }
        list_init(&id->node);

        ctx->cb = fn;
        ctx->udata = this;
        ctx->id = id;

        std::unique_lock<std::mutex> lck(cos_mtx_);
        co_t *co = co_new(sch_, co_task, stack, co_quit, ctx);

        if (!co) {
            free_(ctx);
            free_(id);
            return;
        }

        id->co = co;

        list_add_tail(&id->node, &cos_);
        if (!cur_) {
            cur_ = &id->node;
        }
    }

    void del(co_t *co, CoContext *ctx) {
        std::unique_lock<std::mutex> lck(cos_mtx_);
        // 切到下个
        list_node *n = ctx->id->node.next;

        list_del(&ctx->id->node);
        free_(ctx->id);
        ctx->id = nullptr;

        if (list_empty(&cos_)) {
            cur_ = nullptr;
        } else if (n == &cos_) {
            cur_ = n->next;
        } else {
            cur_ = n;
        }
    }

    void yield(void) {
        assert(cur_);
        co_yield(sch_);
    }

    static int entry(co_schedule_t *sch, void *ud) {
        assert(sch);
        assert(ud);

        CoSchedule *ptr = (CoSchedule*)ud;

        while (!ptr->stop_) {
            while (!ptr->stop_ && co_num(ptr->sch_)) {
                // 获取当前协程
                list_node *node = nullptr;
                {
                    std::unique_lock<std::mutex> lck(ptr->cos_mtx_);
                    node = ptr->cur_;
                }

                if (node) {
                    co_resume(sch, list_entry(node, co_id_t, node)->co);
                    // 切换到下个协程
                    std::unique_lock<std::mutex> lck(ptr->cos_mtx_);
                    if (list_empty(&ptr->cos_)) {
                        ptr->cur_ = nullptr;
                        break;
                    }
                    // 节点没有被切换，只有该协程不是正常退出时才没有切换
                    if (node == ptr->cur_) {
                        list_node *n = node->next;
                        // 最后一个节点
                        if (n == &ptr->cos_) {
                            ptr->cur_ = n->next;
                        } else {
                            ptr->cur_ = n;
                        }
                    }
                }
            }
        }

        {
            std::unique_lock<std::mutex> lck(ptr->mtx_);
            ptr->stop_ = false;
            ptr->stoped_ = true;
            ptr->cond_.notify_all();
        }

        return 0;
    }

    static void co_task(co_schedule_t *sch, void *data) {
        CoContext *ctx = (CoContext*)data;
        ctx->cb();
    }

    static void co_quit(co_t *co, void *data) {
        CoContext *ctx = (CoContext*)data;
        CoSchedule *sch = (CoSchedule*)(ctx->udata);
        sch->del(co, ctx);
        sch->free_(ctx);
    }

private:
    co_schedule_t *sch_;
    bool stop_;
    bool stoped_;
    std::mutex mtx_;
    std::condition_variable cond_;
    std::mutex cos_mtx_;
    co_malloc_t malloc_;
    co_memalign_t memalign_;
    co_free_t free_;
    list_node *cur_;
    struct list_head cos_;
};

CoSchedule *sch_ref(size_t def_stack, malloc_t mc, memalign_t mcalign, free_t fr) {
    thread_local CoSchedule sch;
    thread_local bool init = false;

    if (!init) {
        sch.init(def_stack, mc, mcalign, fr);
        init = true;
    }

    return &sch;
}

void sch_run(CoSchedule *sch) {
    if (sch) {
        sch->run();
    } else {
        sch_ref()->run();
    }
}

void sch_stop(CoSchedule *sch) {
    if (sch) {
        sch->stop();
    }
}

void __add_co(size_t stack, proxy_co_fn fn) {
    sch_ref()->add(stack, fn);
}

void yield(void) {
    sch_ref()->yield();
}

} // namespace sdk

} // namespace aru
