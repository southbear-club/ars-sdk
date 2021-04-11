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
 * @file slab.hpp
 * @brief 
 * @author wotsen (astralrovers@outlook.com)
 * @version 1.0.0
 * @date 2021-04-04
 * 
 * @copyright MIT
 * 
 */
#pragma once

#include <stdint.h>
#include "aru/sdk/lock/lock.hpp"

namespace aru {

namespace sdk {

typedef struct slab_pool_s slab_pool_t;
class MemorySlabImpl;

typedef struct {
    size_t pool_size, used_size, used_pct;
    size_t pages, free_page;
    size_t p_small, p_exact, p_big, p_page; /* 四种slab占用的page数 */
    size_t b_small, b_exact, b_big, b_page; /* 四种slab占用的byte数 */
    size_t max_free_pages;                  /* 最大的连续可用page数 */
} slab_stat_t;

class MemorySlab {
public:
    MemorySlab(void *addr, size_t len, ILock *mem_lock,
               uint8_t min_size_shift = 3);
    ~MemorySlab();

    void *alloc(size_t size);
    void free(void *p);
    void stat(slab_stat_t &st);

private:
    MemorySlabImpl *impl_;
};

}  // namespace sdk

}  // namespace aru
