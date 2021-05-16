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
 * @file bitmap.hpp
 * @brief 
 * @author wotsen (astralrovers@outlook.com)
 * @version 1.0.0
 * @date 2021-04-05
 * 
 * @copyright MIT
 * 
 */
#pragma once
#include <stdint.h>
#include <stddef.h>

namespace ars {
    
namespace sdk {

/// 位图操作

/// 清零
void bitmap_zero(uint8_t* bitmap, size_t nbits);
/// 填充全1
void bitmap_fill(uint8_t* bitmap, size_t nbits);
/// 拷贝
void bitmap_copy(uint8_t *bitmap, const uint8_t *src, size_t nbits);

/// 置位
void bitmap_set(uint8_t *bitmap, size_t start, size_t len);
/// 清除
void bitmap_clear(uint8_t *bitmap, size_t start, size_t len);

/// 或
void bitmap_or(uint8_t* result, const uint8_t* src1, const uint8_t* src2, size_t nbits);
/// 与
void bitmap_and(uint8_t* result, const uint8_t* src1, const uint8_t* src2, size_t nbits);
/// 异或
void bitmap_xor(uint8_t* result, const uint8_t* src1, const uint8_t* src2, size_t nbits);

size_t bitmap_count_leading_zero(const uint8_t* bitmap, size_t nbits);
size_t bitmap_count_next_zero(const uint8_t* bitmap, size_t nbits, size_t start);
size_t bitmap_find_first_zero(const uint8_t* bitmap, size_t nbits);
size_t bitmap_find_next_zero(const uint8_t* bitmap, size_t nbits, size_t start);
size_t bitmap_weight(const uint8_t* bitmap, size_t nbits);

/// @return 0-not set, other-set to 1
int bitmap_test_bit(const uint8_t* bitmap, size_t bits);

} // namespace sdk

} // namespace ars
