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
 * @file dl_plugin.cpp
 * @brief 
 * @author  ()
 * @version 1.0.0
 * @date 2021-04-10
 * 
 * @copyright MIT
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ars/sdk/schedule/dl_plugin.hpp"
#include "ars/sdk/memory/mem.hpp"

namespace ars {
    
namespace sdk {
    
dl_plugin_manager_t *dl_plugin_manager_create(void) {
    dl_plugin_manager_t *pm = (dl_plugin_manager_t*)aru_calloc(1, sizeof(dl_plugin_manager_t));
    if (!pm) {
        return nullptr;
    }
    INIT_LIST_HEAD(&pm->plugins);
    return pm;
}

void dl_plugin_manager_destroy(dl_plugin_manager_t *pm) {
    if (!pm) {
        return ;
    }

    aru_free(pm);
}

static void *plugin_get_func(dl_plugin_t *p, const char *name)
{
    if (!p || !name) {
        return nullptr;
    }
    void *q = dlsym(p->handle, name);
    if (!q) {
        return nullptr;
    }
    return q;
}

dl_plugin_t *dl_plugin_lookup(dl_plugin_manager_t *pm, const char *name) {
    if (!pm || !name)
        return nullptr;

    struct list_head* head = nullptr;
    dl_plugin_t* p = nullptr;

    list_for_each(head, &pm->plugins) {
        p = list_entry(head, dl_plugin_t, entry);
        if (0 == strcmp(p->name, name)) {
            return (dl_plugin_t*)plugin_get_func(p, name);
        }
    }

    return nullptr;
}

dl_plugin_t *dl_plugin_load(dl_plugin_manager_t *pm, const char *path, const char *name) {
    dl_plugin_t *sym = NULL;
    dl_plugin_t *p = NULL;
    void *handle = dlopen(path, RTLD_LAZY);
    if (!handle) {
        goto failed;
    }
    p = dl_plugin_lookup(pm, name);
    if (p) {
        return p;
    }
    sym = (dl_plugin_t*)dlsym(handle, name);
    if (!sym) {
        goto failed;
    }
    p = (dl_plugin_t*)aru_calloc(1, sizeof(dl_plugin_t));
    if (!p) {
        goto failed;
    }
    p->handle = handle;
    p->name = strdup(name);
    p->path = strdup(path);
    list_add(&p->entry, &pm->plugins);
    return p;

failed:
    if (handle) dlclose(handle);
    if (p) {
        aru_free(p->name);
        aru_free(p->path);
        aru_free(p);
    }
    return nullptr;
}

void dl_plugin_unload(dl_plugin_manager_t *pm, const char *name) {
    list_node *pos = nullptr;
    list_node *n = nullptr;
    list_for_each_safe(pos, n, &pm->plugins) {
        dl_plugin_t *p = list_entry(pos, dl_plugin_t, entry);
        dlclose(p->handle);
        list_del(&p->entry);
        aru_free(p->name);
        aru_free(p->path);
        aru_free(p);
    }
}

dl_plugin_t *dl_plugin_reload(dl_plugin_manager_t *pm, const char *path, const char *name) {
    dl_plugin_unload(pm, name);
    return dl_plugin_load(pm, path, name);
}

} // namespace sdk

} // namespace ars
