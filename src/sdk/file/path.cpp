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
#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>
#ifdef __APPLE__
#include <mach-o/dyld.h>  // for _NSGetExecutablePath
#endif
#include <sys/stat.h>

namespace aru {

namespace sdk {

static char local_path[ARU_MAX_PATH];

const char* path_basename(const char* filepath) {
    const char* pos = strrchr_dir(filepath);
    return pos ? pos + 1 : filepath;
}

const char* path_suffixname(const char* filename) {
    const char* pos = strrchr_dot(filename);
    return pos ? pos + 1 : "";
}

int mkdir_p(const char* dir) {
    if (access(dir, 0) == 0) {
        return EEXIST;
    }
    char tmp[ARU_MAX_PATH] = "0";
    memcpy(tmp, dir, strlen(dir) < sizeof(tmp) ? strlen(dir) : sizeof(tmp));
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

int rmdir_p(const char* dir) {
    if (access(dir, 0) != 0) {
        return ENOENT;
    }
    if (rmdir(dir) != 0) {
        return EPERM;
    }
    char tmp[ARU_MAX_PATH] = "";
    memcpy(tmp, dir, strlen(dir) < ARU_MAX_PATH ? strlen(dir) : ARU_MAX_PATH);
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

char* strrchr_dot(const char* str) { return (char*)::strrchr(str, '.'); }

char* strrchr_dir(const char* filepath) {
    char* p = (char*)filepath;
    while (*p) ++p;
    while (--p >= filepath) {
        if (*p == '/') return p;
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
        strncpy(buf, pos + 1, size);
    }
    return buf;
}

char* get_run_dir(char* buf, int size) { return getcwd(buf, size); }

std::string clean(const std::string& s) {
    if (s.empty()) return std::string(1, '.');

    std::string r(s.data(), s.size());
    size_t n = s.size();

    bool rooted = s[0] == '/';
    size_t p = rooted ? 1 : 0;       // index for string r
    size_t dotdot = rooted ? 1 : 0;  // index where .. must stop

    for (size_t i = p; i < n;) {
        if (s[i] == '/' || (s[i] == '.' && (i + 1 == n || s[i + 1] == '/'))) {
            // empty or . element
            ++i;

        } else if (s[i] == '.' && s[i + 1] == '.' && (i + 2 == n || s[i + 2] == '/')) {
            // .. element: remove to last /
            i += 2;

            if (p > dotdot) {
                // backtrack
                for (--p; p > dotdot && r[p] != '/'; --p)
                    ;

            } else if (!rooted) {
                // cannot backtrack, but not rooted, so append .. element
                if (p > 0) r[p++] = '/';
                r[p++] = '.';
                r[p++] = '.';
                dotdot = p;
            }

        } else {
            // real path element, add slash if needed
            if ((rooted && p != 1) || (!rooted && p != 0)) {
                r[p++] = '/';
            }

            // copy element until the next /
            for (; i < n && s[i] != '/'; i++) {
                r[p++] = s[i];
            }
        }
    }

    if (p == 0) return std::string(1, '.');
    return r.substr(0, p);
}

std::string base(const std::string& s) {
    if (s.empty()) return std::string(1, '.');

    size_t p = s.size();
    for (; p > 0; p--) {
        if (s[p - 1] != '/') break;
    }
    if (p == 0) return std::string(1, '/');

    std::string x = (p == s.size() ? s : s.substr(0, p));
    size_t c = x.rfind('/');
    return c != x.npos ? x.substr(c + 1) : x;
}

std::string ext(const std::string& s) {
    for (size_t i = s.size() - 1; i != (size_t)-1 && s[i] != '/'; --i) {
        if (s[i] == '.') return s.substr(i);
    }
    return std::string();
}

char *path_pwd(void)
{
    char *tmp = getcwd(local_path, sizeof(local_path));
    if (!tmp) {
        // printf("getcwd failed: %s\n", strerror(errno));
    }
    return local_path;
}

}  // namespace sdk

}  // namespace aru
