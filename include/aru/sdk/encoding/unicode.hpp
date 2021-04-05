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
 * @file unicode.hpp
 * @brief 
 * @author wotsen (astralrovers@outlook.com)
 * @version 1.0.0
 * @date 2021-04-05
 * 
 * @copyright MIT
 * 
 */
#pragma once
#include <wchar.h>

namespace aru {

namespace sdk {

/// Unicode字符串转换成UTF-8字符串
/// @param[in] src Unicode字符串
/// @param[in] srcLen Unicode字符串长度(length=wcslen(src)), srcLen为0时转换整个字符串(src必须以字符'0'结尾)
/// @param[out] tgt utf8字符串缓冲区
/// @param[in] tgtBytes in-缓冲区长度, 单位字节
/// @return 转换后字符串长度
int unicode_to_utf8(const wchar_t* src, size_t srcLen, char* tgt, size_t tgtBytes);

/// UTF-8字符串转换成Unicode字符串
/// @param[in] src utf-8字符串
/// @param[in] srcLen utf-8字符串长度(length=wcslen(src)), srcLen为0时转换整个字符串(src必须以字符'0'结尾)
/// @param[out] tgt unicode字符串缓冲区
/// @param[in] tgtBytes in-缓冲区长度, 单位字节
/// @return 转换后字符串长度
int unicode_from_utf8(const char* src, size_t srcLen, wchar_t* tgt, size_t tgtBytes);

/// Unicode字符串转换成多字节字符串(Windows平台是Unicode UTF-16)
/// @param[in] src Unicode字符串
/// @param[in] srcLen 字符串长度, n为0时转换整个字符串(此时src必须以字符'0'结尾)
/// @param[out] tgt 多字节字符串缓冲区
/// @param[in] tgtBytes 多字节字符串缓冲区长度, 单位: 字节
/// @return 转换后字符串长度
int unicode_to_mbcs(const wchar_t* src, size_t srcLen, char* tgt, size_t tgtBytes);

/// 多字节字符串转换成Unicode字符串(Windows平台是Unicode UTF-16)
/// @param[in] src 多字节字符串
/// @param[in] srcLen 字符串长度, n为0时转换整个字符串(此时src必须以字符'0'结尾)
/// @param[out] tgt Unicode字符串缓冲区
/// @param[in] tgtBytes Unicode字符串缓冲区长度, 单位: 字节
/// @return 转换后字符串长度
int unicode_from_mbcs(const char* src, size_t srcLen, wchar_t* tgt, size_t tgtBytes);

/// 多字节字符串转换成Unicode字符串(Windows平台是Unicode UTF-16)
/// @param[in] charset 多字节字符串编码类型
/// @param[in] src 多字节字符串
/// @param[in] srcLen 字符串长度, n为0时转换整个字符串(此时src必须以字符'0'结尾)
/// @param[out] tgt Unicode字符串缓冲区
/// @param[in] tgtBytes Unicode字符串缓冲区长度, 单位: 字节
/// @return
//int unicode_encode(const char* charset, const char* src, size_t srcLen, wchar_t* tgt, size_t tgtBytes);

/// Unicode字符串转换成多字节字符串(Windows平台是Unicode UTF-16)
/// @param[in] charset 多字节字符串编码类型
/// @param[in] src Unicode字符串
/// @param[in] srcLen 字符串长度, n为0时转换整个字符串(此时src必须以字符'0'结尾)
/// @param[out] tgt 多字节字符串缓冲区
/// @param[in] tgtBytes 多字节字符串缓冲区长度, 单位: 字节
/// @return
//int unicode_decode(const char* charset, const wchar_t* src, size_t srcLen, char* tgt, size_t tgtBytes);

#define unicode_to_gb2312 aru::sdk::unicode_to_gb18030
#define unicode_from_gb2312 aru::sdk::unicode_from_gb18030
#define unicode_to_gbk aru::sdk::unicode_to_gb18030
#define unicode_from_gbk aru::sdk::unicode_from_gb18030

int unicode_to_gb18030(const wchar_t* src, size_t srcLen, char* tgt, size_t tgtBytes);
int unicode_from_gb18030(const char* src, size_t srcLen, wchar_t* tgt, size_t tgtBytes);

} // namespace sdk

} // namespace aru
