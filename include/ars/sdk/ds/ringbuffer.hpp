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
 * @file ringbuffer.hpp
 * @brief 环形缓冲区
 * @author wotsen (astralrovers@outlook.com)
 * @version 1.0.0
 * @date 2021-04-18
 * 
 * @copyright MIT
 * 
 */
#pragma once

#include <stdlib.h>
#include <sys/types.h>

namespace ars {
    
namespace sdk {

/// 环形缓冲区
typedef struct ringbuffer {
    void *buffer;           ///< 地址
    int length;             ///< 长度
    size_t start;           ///< 起始地址(读)
    size_t end;             ///< 结束地址(写)
} ringbuffer;

/**
 * @brief 创建ringbuffer
 * 
 * @param len 内存长度
 * @return struct ringbuffer* 非NULL成功
 */
struct ringbuffer *rb_create(int len);

/**
 * @brief 销毁
 * 
 * @param rb 句柄
 */
void rb_destroy(struct ringbuffer *rb);

/**
 * @brief 写数据
 * 
 * @param rb 句柄
 * @param buf 数据
 * @param len 长度
 * @return ssize_t 成功写入长度,小于0异常
 */
ssize_t rb_write(struct ringbuffer *rb, const void *buf, size_t len);

/**
 * @brief 取出数据
 * 
 * @param rb 句柄
 * @param buf 数据
 * @param len 长度
 * @return ssize_t 成功读取长度,小于0异常
 */
ssize_t rb_read(struct ringbuffer *rb, void *buf, size_t len);

/**
 * @brief 数据拷贝
 * 
 * @param rb 句柄
 * @param len 实际拷贝的数据长度
 * @return void* 数据地址
 */
void *rb_dump(struct ringbuffer *rb, size_t *len);

/**
 * @brief 数据清空
 * 
 * @param rb 句柄
 */
void rb_cleanup(struct ringbuffer *rb);

/**
 * @brief 剩余的空间
 * 
 * @param rb 
 * @return size_t 长度
 */
size_t rb_get_space_free(struct ringbuffer *rb);

/**
 * @brief 已经使用的空间
 * 
 * @param rb 
 * @return size_t 长度
 */
size_t rb_get_space_used(struct ringbuffer *rb);

} // namespace sdk

} // namespace ars
