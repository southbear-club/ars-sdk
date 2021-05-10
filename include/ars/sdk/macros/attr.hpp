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
	#define ARS_DEPRECATED __declspec(deprecated)
#elif __GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 1)
	#define ARS_DEPRECATED __attribute__((deprecated))
#else
	#define ARS_DEPRECATED
#endif

// 接口导出
#if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
	#define ARS_DLL_EXPORT_API __declspec(dllexport)
	#define ARS_DLL_IMPORT_API __declspec(dllimport)
#else
	#if __GNUC__ >= 4
		#define ARS_DLL_EXPORT_API __attribute__((visibility ("default")))
		#define ARS_DLL_IMPORT_API
	#else
		#define ARS_DLL_EXPORT_API
		#define ARS_DLL_IMPORT_API
	#endif
#endif

// useage:
// #pragma ARS_TODO(something)
#define __ARS_STR2__(x) #x
#define __ARS_STR1__(x) __ARS_STR2__(x)
#define __ARS_LOC__ __FILE__ "(" __ARS_STR1__(__LINE__) "): "

#if _MSC_VER
	#define ARS_TODO(msg) message(__ARS_LOC__ "TODO: " #msg)
	#define ARS_FIXME(msg) message(__ARS_LOC__ "FIXME: " #msg)
#elif __GNUC__
	#define ARS_TODO(msg) message(__ARS_LOC__ "TODO: " #msg)
	#define ARS_FIXME(msg) message(__ARS_LOC__ "FIXME: " #msg)
#else // XCODE
	#define ARS_TODO(msg)	TODO msg
	#define ARS_FIXME(msg)	FIXME msg
#endif

// UNUSED
// #if defined(__cplusplus)
// 	#define ARS_UNUSED(x)
// #else
	#if defined(_MSC_VER)
		#define ARS_UNUSED(x) x
	#elif defined(__GNUC__)
		#define ARS_UNUSED(x) __attribute__((unused)) x 
	#else
		#define ARS_UNUSED(x) x
	#endif
// #endif