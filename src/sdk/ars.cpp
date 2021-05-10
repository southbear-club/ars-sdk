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
 * @file ars.cpp
 * @brief 
 * @author wotsen (astralrovers@outlook.com)
 * @version 1.0.0
 * @date 2021-04-03
 * 
 * @copyright MIT
 * 
 */
#include "ars/sdk/ars.hpp"
#include "configure.h"

namespace ars {
    
int version(void) {
    return (ARS_VERSION_MAJOR << 16) | (ARS_VERSION_MINOR << 8) | ARS_VERSION_ALTER;
}

const char *version_string(void) {
    return ARS_VERSION;
}

uint64_t version_build(void) {
    return ARS_VERSION_BUILD;
}

bool debug_version(void) {
    return ARS_DEBUG;
}

const char *build_time(void) {
    return ARS_BUILD_TIME;
}

const char *version_mode(void) {
    return ARS_MODE;
}

const char *compile_arch(void) {
    return ARS_ARCH;
}

const char *compile_plat(void) {
    return ARS_PLAT;
}

const char *compile_processor(void) {
    return ARS_PROCESSOR;
}

const char *compile_plat_version(void) {
    return ARS_PLAT_VERSION;
}

const char *compile_os(void) {
    return ARS_OS;
}

const char *compiler_tool(void) {
    return ARS_COMPILER;
}

const char *compile_author(void) {
    return ARS_RELEASE_USER;
}

const char *lib_author(void) {
    return ARS_AUTHOR;
}

} // namespace ars

