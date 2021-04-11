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
 * @brief 
 * @author  ()
 * @version 1.0.0
 * @date 2021-04-07
 * 
 * @copyright MIT
 * 
 */
#pragma once

namespace aru {
    
namespace sdk {

///longest common sequence
///@param[in] s1 string source
///@param[in] s2 string source
///@param[out] seq result sequence
///@param[in] len seq buffer size(bytes)
///@return <0-error, 0-ok, >0=need more buffer
int lcs(const char* s1, const char* s2, char* seq, int len);

///longest common substring
///@param[in] s1 string source
///@param[in] s2 string source
///@param[out] sub common substring
///@param[in] len substring buffer size(bytes)
///@return <0-error, 0-ok, >0=need more buffer
int strsubstring(const char* s1, const char* s2, char* sub, int len);

///Knuth-Morris-Pratt Algorithm
///@param[in] s string
///@param[in] pattern substring
///@return 0-can't find substring, other-substring pointer
const char* kmp(const char* s, const char* pattern);

} // namespace sdk

} // namespace aru
