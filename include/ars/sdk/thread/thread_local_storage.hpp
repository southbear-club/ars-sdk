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
 * @file thread_local_storage.hpp
 * @brief 
 * @author wotsen (astralrovers@outlook.com)
 * @version 1.0.0
 * @date 2021-04-05
 * 
 * @copyright MIT
 * 
 */
#pragma once

#include <pthread.h>

namespace ars {

namespace sdk {

typedef pthread_key_t thread_key_t;
#define ARS_INVALID_THREAD_KEY         0xFFFFFFFF
#define ars_thread_key_create(pkey)    pthread_key_create(pkey, NULL)
#define ars_thread_key_delete          pthread_key_delete
#define ars_thread_get_value           pthread_getspecific
#define ars_thread_set_value           pthread_setspecific

class ThreadLocalStorage {
public:
    enum {
        THREAD_NAME = 0,
        EVENT_LOOP  = 1,
        MAX_NUM     = 16,
    };
    ThreadLocalStorage() {
        ars_thread_key_create(&key);
    }

    ~ThreadLocalStorage() {
        ars_thread_key_delete(key);
    }

    void set(void* val) {
        ars_thread_set_value(key, val);
    }

    void* get() {
        return ars_thread_get_value(key);
    }

    static void set(int idx, void* val) {
        return tls[idx].set(val);
    }

    static void* get(int idx) {
        return tls[idx].get();
    }

    static void setThreadName(const char* name) {
        set(THREAD_NAME, (void*)name);
    }
    static const char* threadName();

private:
    thread_key_t key;
    static ThreadLocalStorage tls[MAX_NUM];
};

} // namespace sdk

} // namespace ars
