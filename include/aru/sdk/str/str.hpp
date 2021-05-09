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

/// 留白字符
#define ARU_SPACE_CHARS     " \t\r\n"
/// 成对字符
#define ARU_PAIR_CHARS      "{}[]()<>\"\"\'\'``"

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

/// 转大写
char *strupper(char *str);
/// 转小写
char *strlower(char *str);
/// 逆序
char *strreverse(char *str);

/// 检查字符串前缀
bool strstartswith(const char* str, const char* start);
/// 检查后缀
bool strendswith(const char* str, const char* end);
/// 检查是否包含
bool strcontains(const char* str, const char* sub);

/// 字符串转bool值
/// 1,y,Y,on,true,enable,yes
bool getboolean(const char* str);

/// 字符串格式化
// std::string asprintf(const char* fmt, ...);
std::string format(const char *fmt, ...);

// 以指定字符分割字符串. x,y,z
StringList split(const std::string& str, char delim = ',');

// user=amdin&pswd=123456
KeyValue splitKV(const std::string& str, char kv_kv = '&', char k_v = '=');
/// 去除头尾的空白字符串
std::string trim(const std::string& str, const char* chars = ARU_SPACE_CHARS);
/// 去除左边的空白字符串
std::string trimL(const std::string& str, const char* chars = ARU_SPACE_CHARS);
/// 除去右边的空白字符串
std::string trimR(const std::string& str, const char* chars = ARU_SPACE_CHARS);
/// 去除成对字符
std::string trim_pairs(const std::string& str, const char* pairs = ARU_PAIR_CHARS);
/// 字符串替换
std::string replace(const std::string& str, const std::string& find, const std::string& rep);

/// b=both,l=left,r=right
// strip("abxxa", "ab")       ->  "xx"     strip both left and right.
// strip("abxxa", "ab", 'l')  ->  "xxa"    strip left only.
// strip("abxxa", "ab", 'r')  ->  "abxx"   strip right only.
std::string strip(const char* s, const char* c=" \t\r\n", char d='b');
std::string strip(const char* s, char c, char d = 'b');
std::string strip(const std::string& s, const char* c=" \t\r\n", char d='b');
std::string strip(const std::string& s, char c, char d='b');
std::string strip(const std::string& s, const std::string& c, char d='b');

} // namespace sdk

} // namespace aru
