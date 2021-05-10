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
 * @file str.cpp
 * @brief 
 * @author wotsen (astralrovers@outlook.com)
 * @version 1.0.0
 * @date 2021-04-04
 * 
 * @copyright MIT
 * 
 */
#include "ars/sdk/str/str.hpp"
#include "ars/sdk/macros/defs.hpp"
#include <stddef.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <memory>

namespace ars {

namespace sdk {

char *strupper(char *str) {
    if (str == NULL) return NULL;
    char* p = str;
    while (*p != '\0') {
        if (*p >= 'a' && *p <= 'z') {
            *p &= ~0x20;
        }
        ++p;
    }
    return str;
}

char *strlower(char *str) {
    if (str == NULL) return NULL;
    char* p = str;
    while (*p != '\0') {
        if (*p >= 'A' && *p <= 'Z') {
            *p |= 0x20;
        }
        ++p;
    }
    return str;
}

char *strreverse(char *str) {
    if (str == NULL) return NULL;
    char* b = str;
    char* e = str;
    while(*e) {++e;}
    --e;
    char tmp;
    while (e > b) {
        tmp = *e;
        *e = *b;
        *b = tmp;
        --e;
        ++b;
    }
    return str;
}

bool strstartswith(const char* str, const char* start) {
    assert(str != NULL && start != NULL);
    while (*str && *start && *str == *start) {
        ++str;
        ++start;
    }
    return *start == '\0';
}

bool strendswith(const char* str, const char* end) {
    assert(str != NULL && end != NULL);
    int len1 = 0;
    int len2 = 0;
    while (*str++) {++len1;}
    while (*end++) {++len2;}
    if (len1 < len2) return false;
    while (len2-- > 0) {
        --str;
        --end;
        if (*str != *end) {
            return false;
        }
    }
    return true;
}

bool strcontains(const char* str, const char* sub) {
    assert(str != NULL && sub != NULL);
    return strstr(str, sub) != NULL;
}

bool getboolean(const char* str) {
    if (str == NULL) return false;
    int len = strlen(str);
    if (len == 0) return false;
    switch (len) {
        case 1: return *str == '1' || *str == 'y' || *str == 'Y';
        case 2: return strcasecmp(str, "on") == 0;
        case 3: return strcasecmp(str, "yes") == 0;
        case 4: return strcasecmp(str, "true") == 0;
        case 6: return strcasecmp(str, "enable") == 0;
        default: return false;
    }
}

static inline int vscprintf(const char* fmt, va_list ap) {
    return vsnprintf(NULL, 0, fmt, ap);
}

std::string format(const char *fmt, ...) {
    char buffer[500];
    std::unique_ptr<char[]> release1;
    char *base;
    for (int iter = 0; iter < 2; iter++) {
        int bufsize;
        if (iter == 0) {
            bufsize = sizeof(buffer);
            base = buffer;
        } else {
            bufsize = 30000;
            base = new char[bufsize];
            release1.reset(base);
        }
        char *p = base;
        char *limit = base + bufsize;
        if (p < limit) {
            va_list ap;
            va_start(ap, fmt);
            p += vsnprintf(p, limit - p, fmt, ap);
            va_end(ap);
        }
        // Truncate to available space if necessary
        if (p >= limit) {
            if (iter == 0) {
                continue;  // Try again with larger buffer
            } else {
                p = limit - 1;
                *p = '\0';
            }
        }
        break;
    }
    return base;
}

// x,y,z
StringList split(const std::string& str, char delim) {
    /*
    std::stringstream ss;
    ss << str;
    string item;
    StringList res;
    while (std::getline(ss, item, delim)) {
        res.push_back(item);
    }
    return res;
    */
    const char* p = str.c_str();
    const char* value = p;
    StringList res;
    while (*p != '\0') {
        if (*p == delim) {
            res.push_back(std::string(value, p-value));
            value = p+1;
        }
        ++p;
    }
    res.push_back(value);
    return res;
}
// user=amdin&pswd=123456
KeyValue splitKV(const std::string& str, char kv_kv, char k_v) {
enum {
        s_key,
        s_value,
    } state = s_key;
    const char* p = str.c_str();
    const char* key = p;
    const char* value = NULL;
    int key_len = 0;
    int value_len = 0;
    KeyValue kvs;
    while (*p != '\0') {
        if (*p == kv_kv) {
            if (key_len && value_len) {
                kvs[std::string(key, key_len)] = std::string(value, value_len);
                key_len = value_len = 0;
            }
            state = s_key;
            key = p+1;
        }
        else if (*p == k_v) {
            state = s_value;
            value = p+1;
        }
        else {
            state == s_key ? ++key_len : ++value_len;
        }
        ++p;
    }
    if (key_len && value_len) {
        kvs[std::string(key, key_len)] = std::string(value, value_len);
    }
    return kvs;
}

std::string trim(const std::string& str, const char* chars) {
    std::string::size_type pos1 = str.find_first_not_of(chars);
    if (pos1 == std::string::npos)   return "";

    std::string::size_type pos2 = str.find_last_not_of(chars);
    return str.substr(pos1, pos2-pos1+1);
}

std::string trimL(const std::string& str, const char* chars) {
    std::string::size_type pos = str.find_first_not_of(chars);
    if (pos == std::string::npos)    return "";
    return str.substr(pos);
}

std::string trimR(const std::string& str, const char* chars) {
    std::string::size_type pos = str.find_last_not_of(chars);
    return str.substr(0, pos+1);
}

std::string trim_pairs(const std::string& str, const char* pairs) {
    const char* s = str.c_str();
    const char* e = str.c_str() + str.size() - 1;
    const char* p = pairs;
    bool is_pair = false;
    while (*p != '\0' && *(p+1) != '\0') {
        if (*s == *p && *e == *(p+1)) {
            is_pair = true;
            break;
        }
        p += 2;
    }
    return is_pair ? str.substr(1, str.size()-2) : str;
}

std::string replace(const std::string& str, const std::string& find, const std::string& rep) {
    std::string::size_type pos = 0;
    std::string::size_type a = find.size();
    std::string::size_type b = rep.size();

    std::string res(str);
    while ((pos = res.find(find, pos)) != std::string::npos) {
        res.replace(pos, a, rep);
        pos += b;
    }
    return res;
}

std::string strip(const char* s, const char* c, char d) {
    if (unlikely(!*s)) return std::string();

    char bs[256] = { 0 };
    while (*c) bs[(uint8_t)(*c++)] = 1;

    if (d == 'l' || d == 'L') {
        while (bs[(uint8_t)(*s)]) ++s;
        return std::string(s);

    } else if (d == 'r' || d == 'R') {
        const char* e = s + strlen(s) - 1;
        while (e >= s && bs[(uint8_t)(*e)]) --e;
        return std::string(s, e + 1 - s);

    } else {
        while (bs[(uint8_t)(*s)]) ++s;
        const char* e = s + strlen(s) - 1;
        while (e >= s && bs[(uint8_t)(*e)]) --e;
        return std::string(s, e + 1 - s);
    }
}

std::string strip(const char* s, char c, char d) {
    if (unlikely(!*s)) return std::string();

    if (d == 'l' || d == 'L') {
        while (*s == c) ++s;
        return std::string(s);

    } else if (d == 'r' || d == 'R') {
        const char* e = s + strlen(s) - 1;
        while (e >= s && *e == c) --e;
        return std::string(s, e + 1 - s);

    } else {
        while (*s == c) ++s;
        const char* e = s + strlen(s) - 1;
        while (e >= s && *e == c) --e;
        return std::string(s, e + 1 - s);
    }
}

std::string strip(const std::string& s, const char* c, char d) {
    if (unlikely(s.empty())) return std::string();

    char bs[256] = { 0 };
    while (*c) bs[(uint8_t)(*c++)] = 1;

    if (d == 'l' || d == 'L') {
        size_t b = 0;
        while (b < s.size() && bs[(uint8_t)(s[b])]) ++b;
        return b == 0 ? s : s.substr(b);

    } else if (d == 'r' || d == 'R') {
        size_t e = s.size();
        while (e > 0 && bs[(uint8_t)(s[e - 1])]) --e;
        return e == s.size() ? s : s.substr(0, e);

    } else {
        size_t b = 0, e = s.size();
        while (b < s.size() && bs[(uint8_t)(s[b])]) ++b;
        if (b == s.size()) return std::string();
        while (e > 0 && bs[(uint8_t)(s[e - 1])]) --e;
        return (e - b == s.size()) ? s : s.substr(b, e - b);
    }
}

std::string strip(const std::string& s, char c, char d) {
    if (unlikely(s.empty())) return std::string();

    if (d == 'l' || d == 'L') {
        size_t b = 0;
        while (b < s.size() && s[b] == c) ++b;
        return b == 0 ? s : s.substr(b);

    } else if (d == 'r' || d == 'R') {
        size_t e = s.size();
        while (e > 0 && s[e - 1] == c) --e;
        return e == s.size() ? s : s.substr(0, e);

    } else {
        size_t b = 0, e = s.size();
        while (b < s.size() && s[b] == c) ++b;
        if (b == s.size()) return std::string();
        while (e > 0 && s[e - 1] == c) --e;
        return (e - b == s.size()) ? s : s.substr(b, e - b);
    }
}

std::string strip(const std::string& s, const std::string& c, char d) {
    if (unlikely(s.empty())) return std::string();

    char bs[256] = { 0 };
    for (size_t i = 0; i < c.size(); ++i) {
        bs[(uint8_t)(c[i])] = 1;
    }

    if (d == 'l' || d == 'L') {
        size_t b = 0;
        while (b < s.size() && bs[(uint8_t)(s[b])]) ++b;
        return b == 0 ? s : s.substr(b);

    } else if (d == 'r' || d == 'R') {
        size_t e = s.size();
        while (e > 0 && bs[(uint8_t)(s[e - 1])]) --e;
        return e == s.size() ? s : s.substr(0, e);

    } else {
        size_t b = 0, e = s.size();
        while (b < s.size() && bs[(uint8_t)(s[b])]) ++b;
        if (b == s.size()) return std::string();
        while (e > 0 && bs[(uint8_t)(s[e - 1])]) --e;
        return (e - b == s.size()) ? s : s.substr(b, e - b);
    }
}

} // namespace sdk

} // namespace ars
