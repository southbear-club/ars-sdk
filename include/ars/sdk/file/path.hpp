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
 * @file path.hpp
 * @brief
 * @author wotsen (astralrovers@outlook.com)
 * @version 1.0.0
 * @date 2021-04-04
 *
 * @copyright MIT
 *
 */
#pragma once
#include <string>

namespace ars {

namespace sdk {

#define ARS_MAX_PATH 260

// Return the shortest path name equivalent to path.
std::string clean(const std::string& s);

namespace _xx {
inline std::string join(const std::string& s) { return s; }

template <typename... S>
inline std::string join(const std::string& x, const S&... s) {
    return !x.empty() ? (x + "/" + join(s...)) : join(s...);
}
}  // namespace _xx

inline std::string join(const std::string& s, const std::string& t) {
    std::string v(!s.empty() ? s + "/" + t : t);
    return !v.empty() ? clean(v) : v;
}

// Join any number of path elements into a single path.
// The result is cleaned. In particular, all empty strings are ignored.
template <typename... S>
inline std::string join(const S&... s) {
    std::string v = _xx::join(s...);
    return !v.empty() ? clean(v) : v;
}

// Split path by the final slash, separating it into a dir and file name.
// If there is no slash in path, return an empty dir and file set to path.
// The returned values have the property that path = dir+file.
static inline std::pair<std::string, std::string> path_split(const std::string& s) {
    size_t p = s.rfind('/');
    if (p == s.npos) return std::make_pair(std::string(), s);
    return std::make_pair(s.substr(0, p + 1), s.substr(p + 1));
};

// Return the dir part of the path. The result is cleaned.
// If the path is empty, return ".".
static inline std::string dir(const std::string& s) {
    return clean(path_split(s).first);
}

// str=/mnt/share/image/test.jpg
// basename=test.jpg
// dirname=/mnt/share/image
// filename=test
// suffixname=jpg
std::string basename(const std::string& str);
std::string dirname(const std::string& str);
std::string filename(const std::string& str);
std::string suffixname(const std::string& str);

int mkdir_p(const char* dir);
int rmdir_p(const char* dir);

char *file_path_pwd(void);

char* get_executable_path(char* buf, int size);
char* get_executable_dir(char* buf, int size);
char* get_executable_file(char* buf, int size);
char* get_run_dir(char* buf, int size);

}  // namespace sdk

}  // namespace ars
