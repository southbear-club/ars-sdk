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
 * @file coredump.cpp
 * @brief 
 * @author wotsen (astralrovers@outlook.com)
 * @version 1.0.0
 * @date 2021-04-04
 * 
 * @copyright MIT
 * 
 */
#include "ars/sdk/os/coredump.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <unistd.h>

namespace ars {

namespace sdk {

static const char *const core_format = "core-%e-%p-%t";

bool setup_coredump(const char *path_dir, size_t core_size) {
    struct rlimit rlmt;
    char core_path[1024];

    if (NULL == path_dir) return false;

    if (getrlimit(RLIMIT_CORE, &rlmt) < 0) {
        return false;
    }

    rlmt.rlim_cur = (rlim_t)core_size;
    rlmt.rlim_max = (rlim_t)core_size;

    if (setrlimit(RLIMIT_CORE, &rlmt) < 0) {
        return false;
    }

    if (path_dir[strlen(path_dir) - 1] != '/') {
        sprintf(core_path, "echo %s/%s > /proc/sys/kernel/core_pattern",
                path_dir, core_format);
    } else {
        sprintf(core_path, "echo %s%s > /proc/sys/kernel/core_pattern",
                path_dir, core_format);
    }

    sprintf(core_path, "echo %s/%s > /proc/sys/kernel/core_pattern", path_dir,
            core_format);
    if (system(core_path) != 0) {
        return false;
    }

    if (system("echo 1 > /proc/sys/kernel/core_uses_pid") != 0) {
        return false;
    }

    return true;
}

}  // namespace sdk

}  // namespace ars
