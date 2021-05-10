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

/**
 * @brief 动态库版本
 * 
 */
typedef struct dl_version_s {
    int major;      ///< 主版本号
    int minor;      ///< 次版本号
    int patch;      ///< 补丁号
} dl_version_t;

/// 每个库必须要有这个类型的全局变量来标识这个插件
typedef struct {
    char *name;                         ///< 插件名称，用来标识该插件的全局变量名
    char *path;                         ///< 路径
    dl_version_t version;               ///< 版本

    void *(*open)(void *arg);           ///< 打开时调用
    void (*close)(void *arg);           ///< 关闭时调用
    void *(*call)(void *arg0, ...);     ///< 执行启动

    void *handle;                       ///< dl句柄
    struct list_head entry;
} dl_plugin_t;

/// 插件管理器
typedef struct {
    struct list_head plugins;
} dl_plugin_manager_t;

/**
 * @brief 创建管理器
 * 
 * @return dl_plugin_manager_t* 
 */
dl_plugin_manager_t *dl_plugin_manager_create(void);

/**
 * @brief 删除管理器
 * 
 */
void dl_plugin_manager_destroy(dl_plugin_manager_t *);

// 查找已经加载的动态库的插件
dl_plugin_t *dl_plugin_lookup(dl_plugin_manager_t *pm, const char *name);

/**
 * @brief 加载插件
 * 
 * @param pm    管理器
 * @param path  库路径
 * @param name  名称
 * @return dl_plugin_t* 插件句柄
 */
dl_plugin_t *dl_plugin_load(dl_plugin_manager_t *pm, const char *path, const char *name);

/**
 * @brief 卸载插件
 * 
 * @param pm 管理器
 * @param name 插件名
 */
void dl_plugin_unload(dl_plugin_manager_t *pm, const char *name);

/**
 * @brief 插件重载
 * 
 * @param pm 管理器
 * @param path 路径
 * @param name 名称
 * @return dl_plugin_t* 
 */
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
