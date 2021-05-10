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
 * @file html-entities.hpp
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

namespace ars {
    
namespace sdk {

/// Get HTML entities count
/// @return html entities count
int html_entities_count(void);

/// Get HTML entities
/// @param[in] index from 0 to count-1
/// @param[out] name entities name
/// @param[out] number entities number
void html_entities_get(int index, char name[16], wchar_t *number);

/// decode HTML(UTF-8) content("&lt;" -> "<")
/// @param[out] dst target string buffer, dst length must > src length
/// @param[in] src source string
/// @param[in] srcLen source string length(in bytes)
/// @return >=0-destination string length, <0-error
int html_entities_decode(char* dst, const char* src, int srcLen);

/// encode HTML(UTF-8) content("<" -> "&lt;")
/// @param[out] dst target string buffer, dst length must enough
/// @param[in] src source string
/// @param[in] srcLen source string length(in bytes)
/// @return >=0-destination string length, <0-error
int html_entities_encode(char* dst, const char* src, int srcLen);

} // namespace sdk

} // namespace ars
