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
 * @file dir.hpp
 * @brief 文件夹操作
 * @author wotsen (astralrovers@outlook.com)
 * @version 1.0.0
 * @date 2021-04-05
 * 
 * @copyright MIT
 * 
 */
#pragma once

#include <string.h>
#include <time.h>
#include <stdint.h>
#include <list>

namespace ars {

namespace sdk {

/// 目录对象
typedef struct dir_s {
    char    name[256];
    char    type; // f:file d:dir l:link b:block c:char s:socket p:pipe
    char    reserverd;
    unsigned short mode;
    size_t  size;
    time_t  atime;
    time_t  mtime;
    time_t  ctime;
} dir_t;

/// 列出目录下的文件
int listdir(const char* dir, std::list<dir_t>& dirs);

/// 目录大小
int dir_size(const char *path, uint64_t *size);

/// 目录下的文件数
int num_in_dir(const char *path);

} // namespace sdk

} // namespace ars
