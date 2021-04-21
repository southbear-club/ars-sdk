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
 * @file serializer.hpp
 * @brief 
 * @author wotsen (astralrovers@outlook.com)
 * @version 1.0.0
 * @date 2021-04-18
 * 
 * @copyright MIT
 * 
 */
#pragma once
#include "darray.hpp"

namespace aru {
    
namespace sdk {

struct serializer {
    void *data;
    size_t  (*read)(void *, void *, size_t);
    size_t  (*write)(void *, const void *, size_t);
    size_t  (*getpos)(void *);
    void    (*free)(void *);
};

int serializer_array_init(struct serializer *s);
void serializer_array_deinit(struct serializer *s);
int serializer_array_get_data(struct serializer *s, uint8_t **output, size_t *size);
void serializer_array_reset(struct serializer *s);

int serializer_file_init(struct serializer *s, const char *path);
void serializer_file_deinit(struct serializer *s);

size_t s_read(struct serializer *s, void *data, size_t size);
size_t s_write(struct serializer *s, const void *data, size_t size);
size_t s_getpos(struct serializer *s);

void s_w8(struct serializer *s, uint8_t u8);
void s_wl16(struct serializer *s, uint16_t u16);
void s_wl24(struct serializer *s, uint32_t u24);
void s_wl32(struct serializer *s, uint32_t u32);
void s_wl64(struct serializer *s, uint64_t u64);
void s_wlf(struct serializer *s, float f);
void s_wld(struct serializer *s, double d);
void s_wb16(struct serializer *s, uint16_t u16);
void s_wb24(struct serializer *s, uint32_t u24);
void s_wb32(struct serializer *s, uint32_t u32);
void s_wb64(struct serializer *s, uint64_t u64);
void s_wbf(struct serializer *s, float f);
void s_wbd(struct serializer *s, double d);

} // namespace sdk

} // namespace aru
