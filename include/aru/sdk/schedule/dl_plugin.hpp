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
 * @file dl_plugin.hpp
 * @brief 
 * @author  ()
 * @version 1.0.0
 * @date 2021-04-10
 * 
 * @copyright MIT
 * 
 */
#pragma once
#include "aru/sdk/ds/list.hpp"
#include "aru/sdk/macros/defs.hpp"
#include "dl.hpp"

namespace aru {
    
namespace sdk {

// ref gear-lib --> ligplugin.h

typedef struct dl_version_s {
    int major;
    int minor;
    int patch;
} dl_version_t;

// 每个库必须要有这个类型的全局变量来标识这个插件
typedef struct {
    char *name;
    char *path;
    dl_version_t version;

    void *(*open)(void *arg);
    void (*close)(void *arg);
    void *(*call)(void *arg0, ...);

    void *handle;
    struct list_head entry;
} dl_plugin_t;

typedef struct {
    struct list_head plugins;
} dl_plugin_manager_t;

dl_plugin_manager_t *dl_plugin_manager_create(void);
void dl_plugin_manager_destroy(dl_plugin_manager_t *);

// 查找已经加载的动态库的插件
dl_plugin_t *dl_plugin_lookup(dl_plugin_manager_t *pm, const char *name);

dl_plugin_t *dl_plugin_load(dl_plugin_manager_t *pm, const char *path, const char *name);
void dl_plugin_unload(dl_plugin_manager_t *pm, const char *name);
dl_plugin_t *dl_plugin_reload(dl_plugin_manager_t *pm, const char *path, const char *name);

/*
 * using HOOK_CALL(func, args...), prev/post functions can be hook into func
 */
#define ARU_PLUGIN_HOOK_CALL(fn, ...)                          \
    ({                                                         \
        fn##_prev(__VA_ARGS__);                                \
        ARU_TYPEOF(fn) *sym = aru::sdk::dlsym(RTLD_NEXT, #fn); \
        if (!sym) {                                            \
            return NULL;                                       \
        }                                                      \
        sym(__VA_ARGS__);                                      \
        fn##_post(__VA_ARGS__);                                \
    })

/*
 * using CALL(fn, args...), you need override api
 */
#define ARU_PLUGIN_CALL(fn, ...)                                                 \
    ({                                                                           \
        ARU_TYPEOF(fn) *sym = (ARU_TYPEOF(fn) *)aru::sdk::dlsym(RTLD_NEXT, #fn); \
        sym(__VA_ARGS__);                                                        \
    })

} // namespace sdk

} // namespace aru
