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
 * @file uuid.hpp
 * @brief 
 * @author wotsen (astralrovers@outlook.com)
 * @version 1.0.0
 * @date 2021-04-05
 * 
 * @copyright MIT
 * 
 */
#pragma once
#include <stdio.h>

namespace ars {
    
namespace sdk {

/**
 * @brief 简单生成uuid
 * 
 * @param s 
 */
void uuid_generate_simple(char s[37]);

/**
 * @brief 生成uuid,如果是linux系统，则读取文件
 * 
 * @param s 
 */
static inline void uuid_generate(char s[37])
{
#ifndef __APPLE__
	FILE* fp;
	fp = fopen("/proc/sys/kernel/random/uuid", "r");
	if (fp)
	{
		s[36] = '0';
		fread(s, 1, 36, fp);
		fclose(fp);
	}
	else
#endif
	{
		uuid_generate_simple(s);
	}
}
    
} // namespace sdk

} // namespace ars
