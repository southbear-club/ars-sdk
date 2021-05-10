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
 * @file bits.cpp
 * @brief 
 * @author wotsen (astralrovers@outlook.com)
 * @version 1.0.0
 * @date 2021-04-05
 * 
 * @copyright MIT
 * 
 */
#include "ars/sdk/ds/bits.hpp"
#include <assert.h>

namespace ars {

namespace sdk {

#define BIT_NUM_8	8
//#define BIT_NUM (sizeof(char)*8)

void bits_init(struct bits_t* bits, const void* data, size_t size)
{
	bits->data = (uint8_t*)data;
	bits->size = size;
	bits->bits = 0;
	bits->error = 0;
}

int bits_next(struct bits_t* bits)
{
	uint8_t bit;
	assert(bits && bits->data && bits->size > 0);
	if (bits->bits >= bits->size * 8)
	{
		bits->error = -1;
		return 0; // throw exception
	}

	bit = bits->data[bits->bits / 8] & (0x80U >> (bits->bits % 8));
	return bit ? 1 : 0;
}

uint64_t bits_next_n(struct bits_t* bits, int n)
{
	size_t i;
	uint64_t v;

	assert(n > 0 && n <= 64);
	assert(bits && bits->data && bits->size > 0);
	if (bits->bits + n > bits->size * 8 || n > 64 || n < 0)
	{
		bits->error = -1;
		return 0; // throw exception
	}

	v = bits->data[bits->bits / 8] & (0xFFU >> (bits->bits % 8)); // remain valid value
	if (n <= 8 - (int)(bits->bits % 8))
		return v >> (8 - (bits->bits % 8) - n); // shift right value

	n -= 8 - (int)(bits->bits % 8);
	for (i = 1; n >= 8 && bits->bits / 8 + i < bits->size; i++)
	{
		v <<= 8;
		v += bits->data[bits->bits / 8 + i];
		n -= 8;
	}

	if (n > 0)
	{
		v <<= n;
		v += bits->data[bits->bits / 8 + i] >> (8 - n);
	}

	return v;
}

int bits_read(struct bits_t* bits)
{
	int bit;
	bit = bits_next(bits);
	if(0 == bits->error)
		bits->bits += 1; // update offset
	return bit;
}

uint64_t bits_read_n(struct bits_t* bits, int n)
{
	uint64_t bit;
	bit = bits_next_n(bits, n);
	if (0 == bits->error)
		bits->bits += n; // update offset
	return bit;
}

int bits_read_ue(struct bits_t* bits)
{
	int bit;
	int leadingZeroBits = -1;
	for (bit = 0; !bit && -1 != bit; ++leadingZeroBits)
	{
		bit = bits_read(bits);
		assert(0 == bit || 1 == bit);
	}

	bit = 0;
	if (leadingZeroBits > 0)
	{
		assert(leadingZeroBits < 32);
		bit = (int)bits_read_n(bits, leadingZeroBits);
	}
	return (1 << leadingZeroBits) - 1 + bit;
}

int bits_read_se(struct bits_t* bits)
{
	int v = bits_read_ue(bits);
	return (0 == v % 2 ? -1 : 1) * ((v + 1) / 2);
}

int bits_read_te(struct bits_t* bits)
{
	int v = bits_read_ue(bits);
	if (v != 1)
		return v;
	else
		return bits_read(bits) ? 0 : 1;
}

int bits_write(struct bits_t* bits, int v)
{
	assert(bits && bits->data && bits->size > 0);
	if (bits->bits >= bits->size * 8)
	{
		bits->error = -1;
		return -1; // throw exception
	}

	if (v)
		bits->data[bits->bits / 8] |= (0x80U >> (bits->bits % 8));
	bits->bits += 1; // update offset
	return 0;
}

int bits_write_n(struct bits_t* bits, uint64_t v, int n)
{
	int m;
	size_t i;

	assert(n > 0 && n <= 64);
	assert(bits && bits->data && bits->size > 0);
	if (bits->bits + n > bits->size * 8 || n > 64 || n < 0)
	{
		bits->error = -1;
		return -1; // throw exception
	}

	m = n;
	v <<= 64 - n; // left shift to first bit

	bits->data[bits->bits / 8] |= v >> (56 + (bits->bits % 8)); // remain valid value
	v <<= 8 - (bits->bits % 8);
	n -= 8 - (int)(bits->bits % 8);

	for (i = 1; n > 0; i++)
	{
		assert(bits->bits / 8 + i < bits->size);
		bits->data[bits->bits / 8 + i] = (uint8_t)(v >> 56);
		v <<= 8;
		n -= 8;
	}

	bits->bits += m;
	return 0;
}

#if defined(_DEBUG) || defined(DEBUG)
static void bits_test2(void)
{
	struct bits_t bits;
	// 1, 010, 011, 00100, 00101, 00110, 00111, 0001000, 0001001, 0001010, 0001011, 0001100, 0001101, 0001110, 0001111
	const uint8_t data[] = { 0xA6, 0x42, 0x98, 0xE2, 0x04, 0x8A, 0x16, 0x30, 0x68, 0xE1, 0xE0 };
	bits_init(&bits, data, sizeof(data));
	assert(0 == bits_read_ue(&bits));
	assert(1 == bits_read_ue(&bits));
	assert(2 == bits_read_ue(&bits));
	assert(3 == bits_read_ue(&bits));
	assert(4 == bits_read_ue(&bits));
	assert(5 == bits_read_ue(&bits));
	assert(6 == bits_read_ue(&bits));
	assert(7 == bits_read_ue(&bits));
	assert(8 == bits_read_ue(&bits));
	assert(9 == bits_read_ue(&bits));
	assert(10 == bits_read_ue(&bits));
	assert(11 == bits_read_ue(&bits));
	assert(12 == bits_read_ue(&bits));
	assert(13 == bits_read_ue(&bits));
	assert(14 == bits_read_ue(&bits));
}

void bits_test(void)
{
	struct bits_t bits;
	const uint8_t data[] = { 0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF, 0xCD, 0xAB };
	bits_init(&bits, data, sizeof(data));
	assert(0 == bits_read(&bits));
	assert(0 == bits_read(&bits));
	assert(0 == bits_read(&bits));
	assert(0 == bits_read(&bits));
	assert(0 == bits_read(&bits));
	assert(0 == bits_read(&bits));
	assert(0 == bits_read(&bits));
	assert(1 == bits_read(&bits));

	assert(0 == bits_read(&bits));
	assert(0 == bits_read(&bits));
	assert(1 == bits_read(&bits));
	assert(0 == bits_read(&bits));
	assert(0 == bits_read(&bits));
	assert(0 == bits_read(&bits));
	assert(1 == bits_read(&bits));
//	assert(1 == bits_read(&bits));

	assert(0x05 == bits_read_n(&bits, 3));
	assert(0x15 == bits_read_n(&bits, 8));
	assert(0x27 == bits_read_n(&bits, 6));
	assert(0x08 == bits_read_n(&bits, 4));
	assert(0x09ABCDEF == bits_read_n(&bits, 28));

	assert(0 == bits_read_n(&bits, 17) && bits.error);
	bits_test2();
}
#endif

} // namespace sdk

} // namespace ars
