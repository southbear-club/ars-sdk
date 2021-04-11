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
 * @file slab.cpp
 * @brief 
 * @author wotsen (astralrovers@outlook.com)
 * @version 1.0.0
 * @date 2021-04-04
 * 
 * @copyright MIT
 * 
 */
#define ARU_LOG_MODULE_NAME "sdk-memory-slab"

#include "aru/sdk/memory/slab.hpp"
#include "log/in_log.hpp"

#include <string.h>
#include <unistd.h>

#include "aru/sdk/macros/defs.hpp"

#define SLAB_LOG(severity) ARU_IN_LOG(severity)

namespace aru {

namespace sdk {

#define NCX_SLAB_PAGE_MASK 3
#define NCX_SLAB_PAGE 0
#define NCX_SLAB_BIG 1
#define NCX_SLAB_EXACT 2
#define NCX_SLAB_SMALL 3

#if (__SIZEOF_LONG__ == 4)

#define NCX_SLAB_PAGE_FREE 0
#define NCX_SLAB_PAGE_BUSY 0xffffffff
#define NCX_SLAB_PAGE_START 0x80000000

#define NCX_SLAB_SHIFT_MASK 0x0000000f
#define NCX_SLAB_MAP_MASK 0xffff0000
#define NCX_SLAB_MAP_SHIFT 16

#define NCX_SLAB_BUSY 0xffffffff

#else

#define NCX_SLAB_PAGE_FREE 0
#define NCX_SLAB_PAGE_BUSY 0xffffffffffffffff
#define NCX_SLAB_PAGE_START 0x8000000000000000

#define NCX_SLAB_SHIFT_MASK 0x000000000000000f
#define NCX_SLAB_MAP_MASK 0xffffffff00000000
#define NCX_SLAB_MAP_SHIFT 32

#define NCX_SLAB_BUSY 0xffffffffffffffff

#endif

typedef struct slab_page_s slab_page_t;

struct slab_page_s {
    uintptr_t slab;
    slab_page_t *next;
    uintptr_t prev;
};

struct slab_pool_s {
    size_t min_size;
    size_t min_shift;

    slab_page_t *pages;
    slab_page_t free;

    uint8_t *start;
    uint8_t *end;

    void *addr;
};

class MemorySlabImpl {
public:
    MemorySlabImpl(void *addr, size_t len, ILock *mem_lock,
                   uint8_t min_size_shift)
        : mtx_(mem_lock) {
        pool_ = (slab_pool_t *)addr;

        pool_->addr = addr;
        pool_->min_shift = min_size_shift;
        pool_->end = (uint8_t *)addr + len;

        init();
    }

    ~MemorySlabImpl() {}

    void *alloc(size_t size) {
        void *p;

        mtx_->lock();
        p = _alloc(size);
        mtx_->unlock();

        return p;
    }

    void free(void *p) {
        mtx_->lock();
        _free(p);
        mtx_->unlock();
    }

    void stat(slab_stat_t &st) {
        uintptr_t m, n, mask, slab;
        uintptr_t *bitmap;
        uintptr_t i, j, map, type, obj_size;
        slab_page_t *page;

        memset(&st, 0, sizeof(slab_stat_t));

        page = pool_->pages;
        st.pages = (pool_->end - pool_->start) / pagesize_;

        for (i = 0; i < st.pages; i++) {
            slab = page->slab;
            type = page->prev & NCX_SLAB_PAGE_MASK;

            switch (type) {
                case NCX_SLAB_SMALL:

                    n = (page - pool_->pages) << pagesize_shift_;
                    bitmap = (uintptr_t *)(pool_->start + n);

                    obj_size = 1 << slab;
                    map = (1 << (pagesize_shift_ - slab)) /
                          (sizeof(uintptr_t) * 8);

                    for (j = 0; j < map; j++) {
                        for (m = 1; m; m <<= 1) {
                            if ((bitmap[j] & m)) {
                                st.used_size += obj_size;
                                st.b_small += obj_size;
                            }
                        }
                    }

                    st.p_small++;

                    break;

                case NCX_SLAB_EXACT:

                    if (slab == NCX_SLAB_BUSY) {
                        st.used_size +=
                            sizeof(uintptr_t) * 8 * slab_exact_size_;
                        st.b_exact += sizeof(uintptr_t) * 8 * slab_exact_size_;
                    } else {
                        for (m = 1; m; m <<= 1) {
                            if (slab & m) {
                                st.used_size += slab_exact_size_;
                                st.b_exact += slab_exact_size_;
                            }
                        }
                    }

                    st.p_exact++;

                    break;

                case NCX_SLAB_BIG:

                    j = pagesize_shift_ - (slab & NCX_SLAB_SHIFT_MASK);
                    j = 1 << j;
                    j = ((uintptr_t)1 << j) - 1;
                    mask = j << NCX_SLAB_MAP_SHIFT;
                    obj_size = 1 << (slab & NCX_SLAB_SHIFT_MASK);

                    for (m = (uintptr_t)1 << NCX_SLAB_MAP_SHIFT; m & mask;
                         m <<= 1) {
                        if ((page->slab & m)) {
                            st.used_size += obj_size;
                            st.b_big += obj_size;
                        }
                    }

                    st.p_big++;

                    break;

                case NCX_SLAB_PAGE:

                    if (page->prev == NCX_SLAB_PAGE) {
                        slab = slab & ~NCX_SLAB_PAGE_START;
                        st.used_size += slab * pagesize_;
                        st.b_page += slab * pagesize_;
                        st.p_page += slab;

                        i += (slab - 1);

                        break;
                    }

                default:

                    if (slab > st.max_free_pages) {
                        st.max_free_pages = page->slab;
                    }

                    st.free_page += slab;

                    i += (slab - 1);

                    break;
            }

            page = pool_->pages + i + 1;
        }

        st.pool_size = pool_->end - pool_->start;
        st.used_pct = st.used_size * 100 / st.pool_size;

        // LOG(INFO) << "pool_size : " << st.pool_size << " bytes\n";
        // SLAB_LOG(INFO) << "used_size : " << st.used_size << " bytes\n";
        // SLAB_LOG(INFO) << "used_pct : " << st.used_pct << " bytes\n";

        // SLAB_LOG(INFO) << "total page count : " << st.pages << "\n";
        // SLAB_LOG(INFO) << "free page count : " << st.free_page << "\n";

        // SLAB_LOG(INFO) << "small slab use page : " << st.p_small
        //           << ", \tbytes : " << st.b_small << "\n";
        // SLAB_LOG(INFO) << "exact slab use page : " << st.p_exact
        //           << ", \tbytes : " << st.b_exact << "\n";
        // SLAB_LOG(INFO) << "big slab use page : " << st.p_big
        //           << ", \tbytes : " << st.b_big << "\n";
        // SLAB_LOG(INFO) << "page slab use page : " << st.p_page
        //           << ", \tbytes : " << st.b_page << "\n";

        // SLAB_LOG(INFO) << "max free pages : " << st.max_free_pages << "\n";
    }

private:
    void *_alloc(size_t size) {
        size_t s;
        uintptr_t p, n, m, mask, *bitmap;
        uintptr_t i, slot, shift, map;
        slab_page_t *page, *prev, *slots;

        if (size >= slab_max_size_) {
            page = alloc_pages((size >> pagesize_shift_) +
                               ((size % pagesize_) ? 1 : 0));
            if (page) {
                p = (page - pool_->pages) << pagesize_shift_;
                p += (uintptr_t)pool_->start;

            } else {
                p = 0;
            }

            goto done;
        }

        if (size > pool_->min_size) {
            shift = 1;
            for (s = size - 1; s >>= 1; shift++) { /* void */
            }
            slot = shift - pool_->min_shift;

        } else {
            size = pool_->min_size;
            shift = pool_->min_shift;
            slot = 0;
        }

        slots = (slab_page_t *)((uint8_t *)pool_ + sizeof(slab_pool_t));
        page = slots[slot].next;

        if (page->next != page) {
            if (shift < slab_exact_shift_) {
                do {
                    p = (page - pool_->pages) << pagesize_shift_;
                    bitmap = (uintptr_t *)(pool_->start + p);

                    map = (1 << (pagesize_shift_ - shift)) /
                          (sizeof(uintptr_t) * 8);

                    for (n = 0; n < map; n++) {
                        if (bitmap[n] != NCX_SLAB_BUSY) {
                            for (m = 1, i = 0; m; m <<= 1, i++) {
                                if ((bitmap[n] & m)) {
                                    continue;
                                }

                                bitmap[n] |= m;

                                i = ((n * sizeof(uintptr_t) * 8) << shift) +
                                    (i << shift);

                                if (bitmap[n] == NCX_SLAB_BUSY) {
                                    for (n = n + 1; n < map; n++) {
                                        if (bitmap[n] != NCX_SLAB_BUSY) {
                                            p = (uintptr_t)bitmap + i;

                                            goto done;
                                        }
                                    }

                                    prev = (slab_page_t *)(page->prev &
                                                           ~NCX_SLAB_PAGE_MASK);
                                    prev->next = page->next;
                                    page->next->prev = page->prev;

                                    page->next = NULL;
                                    page->prev = NCX_SLAB_SMALL;
                                }

                                p = (uintptr_t)bitmap + i;

                                goto done;
                            }
                        }
                    }

                    page = page->next;

                } while (page);

            } else if (shift == slab_exact_shift_) {
                do {
                    if (page->slab != NCX_SLAB_BUSY) {
                        for (m = 1, i = 0; m; m <<= 1, i++) {
                            if ((page->slab & m)) {
                                continue;
                            }

                            page->slab |= m;

                            if (page->slab == NCX_SLAB_BUSY) {
                                prev = (slab_page_t *)(page->prev &
                                                       ~NCX_SLAB_PAGE_MASK);
                                prev->next = page->next;
                                page->next->prev = page->prev;

                                page->next = NULL;
                                page->prev = NCX_SLAB_EXACT;
                            }

                            p = (page - pool_->pages) << pagesize_shift_;
                            p += i << shift;
                            p += (uintptr_t)pool_->start;

                            goto done;
                        }
                    }

                    page = page->next;

                } while (page);

            } else { /* shift > slab_exact_shift_ */

                n = pagesize_shift_ - (page->slab & NCX_SLAB_SHIFT_MASK);
                n = 1 << n;
                n = ((uintptr_t)1 << n) - 1;
                mask = n << NCX_SLAB_MAP_SHIFT;

                do {
                    if ((page->slab & NCX_SLAB_MAP_MASK) != mask) {
                        for (m = (uintptr_t)1 << NCX_SLAB_MAP_SHIFT, i = 0;
                             m & mask; m <<= 1, i++) {
                            if ((page->slab & m)) {
                                continue;
                            }

                            page->slab |= m;

                            if ((page->slab & NCX_SLAB_MAP_MASK) == mask) {
                                prev = (slab_page_t *)(page->prev &
                                                       ~NCX_SLAB_PAGE_MASK);
                                prev->next = page->next;
                                page->next->prev = page->prev;

                                page->next = NULL;
                                page->prev = NCX_SLAB_BIG;
                            }

                            p = (page - pool_->pages) << pagesize_shift_;
                            p += i << shift;
                            p += (uintptr_t)pool_->start;

                            goto done;
                        }
                    }

                    page = page->next;

                } while (page);
            }
        }

        page = alloc_pages(1);

        if (page) {
            if (shift < slab_exact_shift_) {
                p = (page - pool_->pages) << pagesize_shift_;
                bitmap = (uintptr_t *)(pool_->start + p);

                s = 1 << shift;
                n = (1 << (pagesize_shift_ - shift)) / 8 / s;

                if (n == 0) {
                    n = 1;
                }

                bitmap[0] = (2 << n) - 1;

                map =
                    (1 << (pagesize_shift_ - shift)) / (sizeof(uintptr_t) * 8);

                for (i = 1; i < map; i++) {
                    bitmap[i] = 0;
                }

                page->slab = shift;
                page->next = &slots[slot];
                page->prev = (uintptr_t)&slots[slot] | NCX_SLAB_SMALL;

                slots[slot].next = page;

                p = ((page - pool_->pages) << pagesize_shift_) + s * n;
                p += (uintptr_t)pool_->start;

                goto done;

            } else if (shift == slab_exact_shift_) {
                page->slab = 1;
                page->next = &slots[slot];
                page->prev = (uintptr_t)&slots[slot] | NCX_SLAB_EXACT;

                slots[slot].next = page;

                p = (page - pool_->pages) << pagesize_shift_;
                p += (uintptr_t)pool_->start;

                goto done;

            } else { /* shift > slab_exact_shift_ */

                page->slab = ((uintptr_t)1 << NCX_SLAB_MAP_SHIFT) | shift;
                page->next = &slots[slot];
                page->prev = (uintptr_t)&slots[slot] | NCX_SLAB_BIG;

                slots[slot].next = page;

                p = (page - pool_->pages) << pagesize_shift_;
                p += (uintptr_t)pool_->start;

                goto done;
            }
        }

        p = 0;

    done:

        return (void *)p;
    }

    void _free(void *p) {
        size_t size;
        uintptr_t slab, m, *bitmap;
        uintptr_t n, type, slot, shift, map;
        slab_page_t *slots, *page;

        if ((uint8_t *)p < pool_->start || (uint8_t *)p > pool_->end) {
            // SLAB_LOG(ERROR) << "outside of pool\n";
            goto fail;
        }

        n = ((uint8_t *)p - pool_->start) >> pagesize_shift_;
        page = &pool_->pages[n];
        slab = page->slab;
        type = page->prev & NCX_SLAB_PAGE_MASK;

        switch (type) {
            case NCX_SLAB_SMALL:

                shift = slab & NCX_SLAB_SHIFT_MASK;
                size = 1 << shift;

                if ((uintptr_t)p & (size - 1)) {
                    goto wrong_chunk;
                }

                n = ((uintptr_t)p & (pagesize_ - 1)) >> shift;
                m = (uintptr_t)1 << (n & (sizeof(uintptr_t) * 8 - 1));
                n /= (sizeof(uintptr_t) * 8);
                bitmap = (uintptr_t *)((uintptr_t)p & ~(pagesize_ - 1));

                if (bitmap[n] & m) {
                    if (page->next == NULL) {
                        slots = (slab_page_t *)((uint8_t *)pool_ +
                                                sizeof(slab_pool_t));
                        slot = shift - pool_->min_shift;

                        page->next = slots[slot].next;
                        slots[slot].next = page;

                        page->prev = (uintptr_t)&slots[slot] | NCX_SLAB_SMALL;
                        page->next->prev = (uintptr_t)page | NCX_SLAB_SMALL;
                    }

                    bitmap[n] &= ~m;

                    n = (1 << (pagesize_shift_ - shift)) / 8 / (1 << shift);

                    if (n == 0) {
                        n = 1;
                    }

                    if (bitmap[0] & ~(((uintptr_t)1 << n) - 1)) {
                        goto done;
                    }

                    map = (1 << (pagesize_shift_ - shift)) /
                          (sizeof(uintptr_t) * 8);

                    for (n = 1; n < map; n++) {
                        if (bitmap[n]) {
                            goto done;
                        }
                    }

                    free_pages(page, 1);

                    goto done;
                }

                goto chunk_already_free;

            case NCX_SLAB_EXACT:

                m = (uintptr_t)1
                    << (((uintptr_t)p & (pagesize_ - 1)) >> slab_exact_shift_);
                size = slab_exact_size_;

                if ((uintptr_t)p & (size - 1)) {
                    goto wrong_chunk;
                }

                if (slab & m) {
                    if (slab == NCX_SLAB_BUSY) {
                        slots = (slab_page_t *)((uint8_t *)pool_ +
                                                sizeof(slab_pool_t));
                        slot = slab_exact_shift_ - pool_->min_shift;

                        page->next = slots[slot].next;
                        slots[slot].next = page;

                        page->prev = (uintptr_t)&slots[slot] | NCX_SLAB_EXACT;
                        page->next->prev = (uintptr_t)page | NCX_SLAB_EXACT;
                    }

                    page->slab &= ~m;

                    if (page->slab) {
                        goto done;
                    }

                    free_pages(page, 1);

                    goto done;
                }

                goto chunk_already_free;

            case NCX_SLAB_BIG:

                shift = slab & NCX_SLAB_SHIFT_MASK;
                size = 1 << shift;

                if ((uintptr_t)p & (size - 1)) {
                    goto wrong_chunk;
                }

                m = (uintptr_t)1
                    << ((((uintptr_t)p & (pagesize_ - 1)) >> shift) +
                        NCX_SLAB_MAP_SHIFT);

                if (slab & m) {
                    if (page->next == NULL) {
                        slots = (slab_page_t *)((uint8_t *)pool_ +
                                                sizeof(slab_pool_t));
                        slot = shift - pool_->min_shift;

                        page->next = slots[slot].next;
                        slots[slot].next = page;

                        page->prev = (uintptr_t)&slots[slot] | NCX_SLAB_BIG;
                        page->next->prev = (uintptr_t)page | NCX_SLAB_BIG;
                    }

                    page->slab &= ~m;

                    if (page->slab & NCX_SLAB_MAP_MASK) {
                        goto done;
                    }

                    free_pages(page, 1);

                    goto done;
                }

                goto chunk_already_free;

            case NCX_SLAB_PAGE:

                if ((uintptr_t)p & (pagesize_ - 1)) {
                    goto wrong_chunk;
                }

                if (slab == NCX_SLAB_PAGE_FREE) {
                    // SLAB_LOG(WARNING) << "page is already free\n";
                    goto fail;
                }

                if (slab == NCX_SLAB_PAGE_BUSY) {
                    // SLAB_LOG(WARNING) << "pointer to wrong page\n";
                    goto fail;
                }

                n = ((uint8_t *)p - pool_->start) >> pagesize_shift_;
                size = slab & ~NCX_SLAB_PAGE_START;

                free_pages(&pool_->pages[n], size);

                return;
        }

        /* not reached */

        return;

    done:

        return;

    wrong_chunk:

        // SLAB_LOG(ERROR) << "pointer to wrong chunk\n";

        goto fail;

    chunk_already_free:

        // SLAB_LOG(ERROR) << "chunk is already free\n";

    fail:

        return;
    }

    void init(void) {
        uint8_t *p;
        size_t size;
        uintptr_t n, pages;
        slab_page_t *slots;

        /*pagesize*/
        pagesize_ = getpagesize();
        for (n = pagesize_, pagesize_shift_ = 0; n >>= 1;
             pagesize_shift_++) { /* void */
        }

        slab_max_size_ = pagesize_ / 2;
        slab_exact_size_ = pagesize_ / (8 * sizeof(uintptr_t));
        for (n = slab_exact_size_; n >>= 1; slab_exact_shift_++) {
            /* void */
        }

        pool_->min_size = 1 << pool_->min_shift;

        p = (uint8_t *)pool_ + sizeof(slab_pool_t);
        slots = (slab_page_t *)p;

        n = pagesize_shift_ - pool_->min_shift;
        for (uintptr_t i = 0; i < n; i++) {
            slots[i].slab = 0;
            slots[i].next = &slots[i];
            slots[i].prev = 0;
        }

        p += n * sizeof(slab_page_t);

        size = pool_->end - p;

        pages = (uintptr_t)(size / (pagesize_ + sizeof(slab_page_t)));

        memset(p, 0, pages * sizeof(slab_page_t));

        pool_->pages = (slab_page_t *)p;

        pool_->free.prev = 0;
        pool_->free.next = (slab_page_t *)p;

        pool_->pages->slab = pages;
        pool_->pages->next = &pool_->free;
        pool_->pages->prev = (uintptr_t)&pool_->free;

        pool_->start = (uint8_t *)aru_align_ptr(
            (uintptr_t)p + pages * sizeof(slab_page_t), pagesize_);

        real_pages_ = (pool_->end - pool_->start) / pagesize_;
        pool_->pages->slab = real_pages_;
    }

    slab_page_t *alloc_pages(uintptr_t pages) {
        slab_page_t *page, *p;

        for (page = pool_->free.next; page != &pool_->free; page = page->next) {
            if (page->slab >= pages) {
                if (page->slab > pages) {
                    page[pages].slab = page->slab - pages;
                    page[pages].next = page->next;
                    page[pages].prev = page->prev;

                    p = (slab_page_t *)page->prev;
                    p->next = &page[pages];
                    page->next->prev = (uintptr_t)&page[pages];

                } else {
                    p = (slab_page_t *)page->prev;
                    p->next = page->next;
                    page->next->prev = page->prev;
                }

                page->slab = pages | NCX_SLAB_PAGE_START;
                page->next = NULL;
                page->prev = NCX_SLAB_PAGE;

                if (--pages == 0) {
                    return page;
                }

                for (p = page + 1; pages; pages--) {
                    p->slab = NCX_SLAB_PAGE_BUSY;
                    p->next = NULL;
                    p->prev = NCX_SLAB_PAGE;
                    p++;
                }

                return page;
            }
        }

        // SLAB_LOG(ERROR) << "failed no memory\n";

        return NULL;
    }

    void free_pages(slab_page_t *page, uintptr_t pages) {
        slab_page_t *prev, *next;

        if (pages > 1) {
            memset(&page[1], 0, (pages - 1) * sizeof(slab_page_t));
        }

        if (page->next) {
            prev = (slab_page_t *)(page->prev & ~NCX_SLAB_PAGE_MASK);
            prev->next = page->next;
            page->next->prev = page->prev;
        }

        page->slab = pages;
        page->prev = (uintptr_t)&pool_->free;
        page->next = pool_->free.next;
        page->next->prev = (uintptr_t)page;

        pool_->free.next = page;

        // merge
        if (pool_->pages != page) {
            prev = page - 1;
            if (slab_empty(prev)) {
                for (; prev >= pool_->pages; prev--) {
                    if (prev->slab != 0) {
                        pool_->free.next = page->next;
                        page->next->prev = (uintptr_t)&pool_->free;

                        prev->slab += pages;
                        memset(page, 0, sizeof(slab_page_t));

                        page = prev;

                        break;
                    }
                }
            }
        }

        if ((page - pool_->pages + page->slab) < real_pages_) {
            next = page + page->slab;
            if (slab_empty(next)) {
                prev = (slab_page_t *)(next->prev);
                prev->next = next->next;
                next->next->prev = next->prev;

                page->slab += next->slab;
                memset(next, 0, sizeof(slab_page_t));
            }
        }
    }

    bool slab_empty(slab_page_t *page) {
        slab_page_t *prev;

        if (page->slab == 0) {
            return true;
        }

        // page->prev == PAGE | SMALL | EXACT | BIG
        if (page->next == NULL) {
            return false;
        }

        prev = (slab_page_t *)(page->prev & ~NCX_SLAB_PAGE_MASK);
        while (prev >= pool_->pages) {
            prev = (slab_page_t *)(prev->prev & ~NCX_SLAB_PAGE_MASK);
        };

        if (prev == &pool_->free) {
            return true;
        }

        return false;
    }

private:
    ILock *mtx_;
    slab_pool_t *pool_;
    uintptr_t slab_max_size_;
    uintptr_t slab_exact_size_;
    uintptr_t slab_exact_shift_;
    uintptr_t pagesize_;
    uintptr_t pagesize_shift_;
    uintptr_t real_pages_;
};

MemorySlab::MemorySlab(void *addr, size_t len, ILock *mem_lock,
                       uint8_t min_size_shift) {
    impl_ = new MemorySlabImpl(addr, len, mem_lock, min_size_shift);
}

MemorySlab::~MemorySlab() { delete impl_; }

void *MemorySlab::alloc(size_t size) { return impl_->alloc(size); }

void MemorySlab::free(void *p) { impl_->free(p); }

void MemorySlab::stat(slab_stat_t &st) { impl_->stat(st); }

}  // namespace sdk

}  // namespace aru
