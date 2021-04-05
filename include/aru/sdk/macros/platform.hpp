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
 * @file platform.hpp
 * @brief 
 * @author wotsen (astralrovers@outlook.com)
 * @version 1.0.0
 * @date 2021-04-05
 * 
 * @copyright MIT
 * 
 */
#pragma once

#if defined(linux) || defined(__linux) || defined(__linux__)
    #define ARU_OS_LINUX
#elif defined(__APPLE__) && (defined(__GNUC__) || defined(__xlC__) || defined(__xlc__))
    #include <TargetConditionals.h>
    #if defined(TARGET_OS_MAC) && TARGET_OS_MAC
        #define ARU_OS_MAC
    #elif defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE
        #define ARU_OS_IOS
    #endif
    #define ARU_OS_DARWIN
#else
#error "Unsupported operating system platform!"
#endif

// ARCH
#if defined(__i386) || defined(__i386__) || defined(_M_IX86)
    #define ARU_ARCH_X86
    #define ARU_ARCH_X86_32
#elif defined(__x86_64) || defined(__x86_64__) || defined(__amd64) || defined(_M_X64)
    #define ARU_ARCH_X64
    #define ARU_ARCH_X86_64
#elif defined(__arm__)
    #define ARU_ARCH_ARM
#elif defined(__aarch64__) || defined(__ARM64__)
    #define ARU_ARCH_ARM64
#else
    #define ARU_ARCH_UNKNOWN
    #warning "Unknown hardware architecture!"
#endif