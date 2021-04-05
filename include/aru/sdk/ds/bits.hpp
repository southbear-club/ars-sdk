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
 * @file bits.hpp
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

namespace aru {

namespace sdk {

struct bits_t
{
	uint8_t* data;
	size_t size;
	size_t bits; // offset bit
	int error;
};

#define bits_read_uint8(bits, n)		(uint8_t)aru::sdk::bits_read_n(bits, n)
#define bits_read_uint16(bits, n)		(uint16_t)aru::sdk::bits_read_n(bits, n)
#define bits_read_uint32(bits, n)		(uint32_t)aru::sdk::bits_read_n(bits, n)
#define bits_read_uint64(bits, n)		(uint64_t)aru::sdk::bits_read_n(bits, n)
#define bits_write_uint8(bits, v, n)	aru::sdk::bits_write_n(bits, (uint64_t)v, n)
#define bits_write_uint16(bits, v, n)	aru::sdk::bits_write_n(bits, (uint64_t)v, n)
#define bits_write_uint32(bits, v, n)	aru::sdk::bits_write_n(bits, (uint64_t)v, n)
#define bits_write_uint64(bits, v, n)	aru::sdk::bits_write_n(bits, (uint64_t)v, n)

void bits_init(struct bits_t* bits, const void* data, size_t size);

/// get 1-bit from bit stream(don't offset position)
/// @param[in] bits bit stream
/// @return -1-error, 1-value, 0-value
int bits_next(struct bits_t* bits);

/// read n-bit(n <= 64) from bit stream(don't offset position)
/// @param[in] bits bit stream
/// @return -1-error, 1-value, 0-value
uint64_t bits_next_n(struct bits_t* bits, int n);

/// read 1-bit from bit stream(offset position)
/// @param[in] bits bit stream
/// @return -1-error, 1-value, 0-value
int bits_read(struct bits_t* bits);

/// read n-bit(n <= 64) from bit stream(offset position)
/// @param[in] bits bit stream
/// @return -1-error, other-value
uint64_t bits_read_n(struct bits_t* bits, int n);

/// Exp-Golomb codes
int bits_read_ue(struct bits_t* bits);
int bits_read_se(struct bits_t* bits);
int bits_read_te(struct bits_t* bits);

/// write 1-bit to bit stream(offset position)
/// @param[in] bits bit stream
/// @param[in] v 0-0, other-1
/// @return 0-ok, other-error
int bits_write(struct bits_t* bits, int v);

/// write n-bit to bit stream(offset position)
/// @param[in] bits bit stream
/// @param[in] v value
/// @param[in] n value bit count
/// @return 0-ok, other-error
int bits_write_n(struct bits_t* bits, uint64_t v, int n);

} // namespace sdk

} // namespace aru
