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
#include "ars/sdk/file/path.hpp"
#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>
#ifdef __APPLE__
#include <mach-o/dyld.h>  // for _NSGetExecutablePath
#endif
#include <sys/stat.h>

namespace ars {

namespace sdk {

static char local_path[ARS_MAX_PATH];

int mkdir_p(const char* dir) {
    if (access(dir, 0) == 0) {
        return EEXIST;
    }
    char tmp[ARS_MAX_PATH] = "0";
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
    char tmp[ARS_MAX_PATH] = "";
    memcpy(tmp, dir, strlen(dir) < ARS_MAX_PATH ? strlen(dir) : ARS_MAX_PATH);
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
    char filepath[ARS_MAX_PATH];
    get_executable_path(filepath, sizeof(filepath));
    std::string dir = dirname(filepath);
    snprintf(buf, size, "%s", dir.c_str());

    return buf;
}

char* get_executable_file(char* buf, int size) {
    char filepath[ARS_MAX_PATH];
    get_executable_path(filepath, sizeof(filepath));
    std::string f = basename(filepath);
    snprintf(buf, size, "%s", f.c_str());

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

char *path_pwd(void)
{
    char *tmp = getcwd(local_path, sizeof(local_path));
    if (!tmp) {
        // printf("getcwd failed: %s\n", strerror(errno));
    }
    return local_path;
}

std::string basename(const std::string& str) {
    std::string::size_type pos1 = str.find_last_not_of("/\\");
    if (pos1 == std::string::npos) {
        return "/";
    }
    std::string::size_type pos2 = str.find_last_of("/\\", pos1);
    if (pos2 == std::string::npos) {
        pos2 = 0;
    } else {
        pos2++;
    }

    return str.substr(pos2, pos1-pos2+1);
}

std::string dirname(const std::string& str) {
    std::string::size_type pos1 = str.find_last_not_of("/\\");
    if (pos1 == std::string::npos) {
        return "/";
    }
    std::string::size_type pos2 = str.find_last_of("/\\", pos1);
    if (pos2 == std::string::npos) {
        return ".";
    } else if (pos2 == 0) {
        pos2 = 1;
    }

    return str.substr(0, pos2);
}

std::string filename(const std::string& str) {
    std::string::size_type pos1 = str.find_last_of("/\\");
    if (pos1 == std::string::npos) {
        pos1 = 0;
    } else {
        pos1++;
    }
    std::string file = str.substr(pos1, -1);

    std::string::size_type pos2 = file.find_last_of(".");
    if (pos2 == std::string::npos) {
        return file;
    }
    return file.substr(0, pos2);
}

std::string suffixname(const std::string& str) {
    std::string::size_type pos1 = str.find_last_of("/\\");
    if (pos1 == std::string::npos) {
        pos1 = 0;
    } else {
        pos1++;
    }
    std::string file = str.substr(pos1, -1);

    std::string::size_type pos2 = file.find_last_of(".");
    if (pos2 == std::string::npos) {
        return "";
    }
    return file.substr(pos2+1, -1);
}

}  // namespace sdk

}  // namespace ars
