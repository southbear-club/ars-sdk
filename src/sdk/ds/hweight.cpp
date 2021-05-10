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
 * @file hweight.cpp
 * @brief 
 * @author wotsen (astralrovers@outlook.com)
 * @version 1.0.0
 * @date 2021-04-05
 * 
 * @copyright MIT
 * 
 */
#include "ars/sdk/ds/hweight.hpp"
#if defined(DEBUG) || defined(_DEBUG)
#include <assert.h>
#endif

namespace ars {
    
namespace sdk {

int hweight8(uint8_t w)
{
	w = w - ((w >> 1) & 0x55);
	w = (w & 0x33) + ((w >> 2) & 0x33);
	return (w + (w >> 4)) & 0x0F;
}

int hweight16(uint16_t w)
{
	w = w - ((w >> 1) & 0x5555);
	w = (w & 0x3333) + ((w >> 2) & 0x3333);
	w = (w  + (w >> 4)) & 0x0F0F;
	return (w + (w >> 8)) & 0x00FF;
}

int hweight32(uint32_t w)
{
	w = w - ((w >> 1) & 0x55555555);
	w = (w & 0x33333333) + ((w >> 2) & 0x33333333);
	w = (w + (w >> 4)) & 0x0F0F0F0F;
	w = (w + (w >> 8));
	return (w + (w >> 16)) & 0x000000FF;
}

int hweight64(uint64_t w)
{
	w = w - ((w >> 1) & 0x5555555555555555ull);
	w = (w & 0x3333333333333333ull) + ((w >> 2) & 0x3333333333333333ull);
	w = (w + (w >> 4)) & 0x0F0F0F0F0F0F0F0Full;
	w = (w + (w >> 8));
	w = (w + (w >> 16));
	return (w + (w >> 32)) & 0x00000000000000FFull;
}

#if defined(DEBUG) || defined(_DEBUG)
void hweight_test(void)
{
	assert(8 == hweight8(0xFF));
	assert(0 == hweight8(0x00));
	assert(2 == hweight8(0x88));
	assert(2 == hweight8(0x11));
	assert(5 == hweight8(0xF8));
	assert(4 == hweight8(0x0F));
	assert(4 == hweight8(0x3C));

	assert(16 == hweight16(0xFFFF));
	assert(0 == hweight16(0x0000));
	assert(4 == hweight16(0x8421));
	assert(4 == hweight16(0x1428));
	assert(10 == hweight16(0xF88F));
	assert(8 == hweight16(0x0FF0));
	assert(8 == hweight16(0x3CC3));

	assert(32 == hweight32(0xFFFFFFFF));
	assert(0 == hweight32(0x00000000));
	assert(8 == hweight32(0x84211248));
	assert(8 == hweight32(0x14282418));
	assert(20 == hweight32(0xF88FF11F));
	assert(16 == hweight32(0x0FF063C9));
	assert(16 == hweight32(0x3CC35699));

	assert(64 == hweight64(0xFFFFFFFFFFFFFFFFull));
	assert(0 == hweight64(0x0000000000000000ull));
	assert(16 == hweight64(0x8421824181428124ull));
	assert(16 == hweight64(0x1248128414281842ull));
	assert(40 == hweight64(0xF8F4F2F1373E57E9ull));
	assert(32 == hweight64(0x0F172E33CC3CC371ull));
	assert(32 == hweight64(0x9CC993396CC66336ull));
}
#endif

} // namespace sdk

} // namespace ars
