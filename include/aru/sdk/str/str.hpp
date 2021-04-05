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
 * @file str.hpp
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
#include <sstream>
#include <map>
#include <vector>
#include <string.h>

namespace aru {

namespace sdk {

typedef std::vector<std::string> StringList;
typedef std::map<std::string, std::string> KeyValue;

// std::map<std::string, std::string, StringCaseLess>
class StringCaseLess : public std::less<std::string> {
public:
    bool operator()(const std::string& lhs, const std::string& rhs) const {
        return strcasecmp(lhs.c_str(), rhs.c_str()) < 0;
    }
};

// NOTE: low-version NDK not provide std::to_string
template<typename T>
static inline std::string to_string(const T& t) {
    std::ostringstream oss;
    oss << t;
    return oss.str();
}

template<typename T>
static inline T from_string(const std::string& str) {
    T t;
    std::istringstream iss(str);
    iss >> t;
    return t;
}

#define ARU_SPACE_CHARS     " \t\r\n"
#define ARU_PAIR_CHARS      "{}[]()<>\"\"\'\'``"

char *strupper(char *str);
char *strlower(char *str);
char *strreverse(char *str);

bool strstartswith(const char* str, const char* start);
bool strendswith(const char* str, const char* end);
bool strcontains(const char* str, const char* sub);

bool getboolean(const char* str);

std::string asprintf(const char* fmt, ...);
// x,y,z
StringList split(const std::string& str, char delim = ',');
// user=amdin&pswd=123456
KeyValue splitKV(const std::string& str, char kv_kv = '&', char k_v = '=');
std::string trim(const std::string& str, const char* chars = ARU_SPACE_CHARS);
std::string trimL(const std::string& str, const char* chars = ARU_SPACE_CHARS);
std::string trimR(const std::string& str, const char* chars = ARU_SPACE_CHARS);
std::string trim_pairs(const std::string& str, const char* pairs = ARU_PAIR_CHARS);
std::string replace(const std::string& str, const std::string& find, const std::string& rep);

// str=/mnt/share/image/test.jpg
// basename=test.jpg
// dirname=/mnt/share/image
// filename=test
// suffixname=jpg
std::string basename(const std::string& str);
std::string dirname(const std::string& str);
std::string filename(const std::string& str);
std::string suffixname(const std::string& str);

} // namespace sdk

} // namespace aru
