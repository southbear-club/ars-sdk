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
 * @file path.cpp
 * @brief 
 * @author wotsen (astralrovers@outlook.com)
 * @version 1.0.0
 * @date 2021-04-04
 * 
 * @copyright MIT
 * 
 */
#include "aru/sdk/file/path.hpp"
#include <stddef.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#ifdef __APPLE__
#include <mach-o/dyld.h> // for _NSGetExecutablePath
#endif
#include <sys/stat.h>

namespace aru {

namespace sdk {

const char *path_basename(const char* filepath) {
    const char* pos = strrchr_dir(filepath);
    return pos ? pos+1 : filepath;
}

const char *path_suffixname(const char* filename) {
    const char* pos = strrchr_dot(filename);
    return pos ? pos+1 : "";
}

int mkdir_p(const char *dir) {
    if (access(dir, 0) == 0) {
        return EEXIST;
    }
    char tmp[ARU_MAX_PATH];
    strncpy(tmp, dir, sizeof(tmp));
    char* p = tmp;
    char delim = '/';
    while (*p) {
        if (*p == '/') {
            *p = '\0';
            mkdir(tmp, 0777);
            *p = delim;
        }
        ++p;
    }
    if (mkdir(tmp, 0777) != 0) {
        return EPERM;
    }
    return 0;
}

int rmdir_p(const char *dir) {
    if (access(dir, 0) != 0) {
        return ENOENT;
    }
    if (rmdir(dir) != 0) {
        return EPERM;
    }
    char tmp[ARU_MAX_PATH];
    strncpy(tmp, dir, sizeof(tmp));
    char* p = tmp;
    while (*p) ++p;
    while (--p >= tmp) {
        if (*p == '/') {
            *p = '\0';
            if (rmdir(tmp) != 0) {
                return 0;
            }
        }
    }
    return 0;
}

char *strrchr_dot(const char *str) {
    return (char *)::strrchr(str, '.');
}

char* strrchr_dir(const char* filepath) {
        char* p = (char*)filepath;
    while (*p) ++p;
    while (--p >= filepath) {
        if (*p == '/')
            return p;
    }
    return NULL;
}

char* get_executable_path(char* buf, int size) {
#if defined(__linux__)
    if (readlink("/proc/self/exe", buf, size) == -1) {
        return NULL;
    }
#elif defined(__APPLE__)
    _NSGetExecutablePath(buf, (uint32_t*)&size);
#endif
    return buf;
}

char* get_executable_dir(char* buf, int size) {
    char filepath[ARU_MAX_PATH];
    get_executable_path(filepath, sizeof(filepath));
    char* pos = strrchr_dir(filepath);
    if (pos) {
        *pos = '\0';
        strncpy(buf, filepath, size);
    }
    return buf;
}

char* get_executable_file(char* buf, int size) {
    char filepath[ARU_MAX_PATH];
    get_executable_path(filepath, sizeof(filepath));
    char* pos = strrchr_dir(filepath);
    if (pos) {
        strncpy(buf, pos+1, size);
    }
    return buf;
}

char* get_run_dir(char* buf, int size) {
    return getcwd(buf, size);
}

} // namespace sdk

} // namespace aru
