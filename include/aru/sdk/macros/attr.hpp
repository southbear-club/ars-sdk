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
 * @file attr.hpp
 * @brief 
 * @author wotsen (astralrovers@outlook.com)
 * @version 1.0.0
 * @date 2021-04-05
 * 
 * @copyright MIT
 * 
 */
#pragma once

// 弃用的接口
#if defined(_MSC_VER)
	#define ARU_DEPRECATED __declspec(deprecated)
#elif __GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 1)
	#define ARU_DEPRECATED __attribute__((deprecated))
#else
	#define ARU_DEPRECATED
#endif

// 接口导出
#if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
	#define ARU_DLL_EXPORT_API __declspec(dllexport)
	#define ARU_DLL_IMPORT_API __declspec(dllimport)
#else
	#if __GNUC__ >= 4
		#define ARU_DLL_EXPORT_API __attribute__((visibility ("default")))
		#define ARU_DLL_IMPORT_API
	#else
		#define ARU_DLL_EXPORT_API
		#define ARU_DLL_IMPORT_API
	#endif
#endif

// useage:
// #pragma ARU_TODO(something)
#define __ARU_STR2__(x) #x
#define __ARU_STR1__(x) __ARU_STR2__(x)
#define __ARU_LOC__ __FILE__ "("__ARU_STR1__(__LINE__)"): "

#if _MSC_VER
	#define ARU_TODO(msg) message(__ARU_LOC__ "TODO: " #msg)
	#define ARU_FIXME(msg) message(__ARU_LOC__ "FIXME: " #msg)
#elif __GNUC__
	#define ARU_TODO(msg) message(__ARU_LOC__ "TODO: " #msg)
	#define ARU_FIXME(msg) message(__ARU_LOC__ "FIXME: " #msg)
#else // XCODE
	#define ARU_TODO(msg)	TODO msg
	#define ARU_FIXME(msg)	FIXME msg
#endif

// UNUSED
#if defined(__cplusplus)
	#define ARU_UNUSED(x)
#else
	#if defined(_MSC_VER)
		#define ARU_UNUSED(x) x
	#elif defined(__GNUC__)
		#define ARU_UNUSED(x) x __attribute__((unused))
	#else
		#define ARU_UNUSED(x) x
	#endif
#endif