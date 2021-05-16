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
 * @file algorithm.hpp
 * @brief 算法相关
 * @author wotsen (astralrovers@outlook.com)
 * @version 1.0.0
 * @date 2021-04-07
 * 
 * @copyright MIT
 * 
 */
#pragma once

namespace ars {
    
namespace sdk {

/**
 * @brief 最长公共子串1
 * 
 * @param s1 字符串1
 * @param s2 字符串2
 * @param seq[out] 字串
 * @param len 子串seq可用的长度
 * @return int 小于0异常，0正常，大于0为seq内存不足
 */
int lcs(const char* s1, const char* s2, char* seq, int len);

/**
 * @brief KMP算法，Knuth-Morris-Pratt
 * 
 * @param s 原字符串
 * @param pattern 匹配串
 * @return const char* NULL-没找到，结果为字串的结果位置
 */
const char* kmp(const char* s, const char* pattern);

} // namespace sdk

} // namespace ars
