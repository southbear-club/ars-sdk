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
 * @file shm.hpp
 * @brief 
 * @author  ()
 * @version 1.0.0
 * @date 2021-04-10
 * 
 * @copyright MIT
 * 
 */
#pragma once
#include <stddef.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

namespace ars {
    
namespace sdk {

namespace ipc {

typedef int shm_t;

static inline shm_t shm_open(const char *name, int oflag, mode_t mode) {
    return ::shm_open(name, oflag, mode);
}

static inline int shm_close(shm_t fd) {
    return ::close(fd);
}

static inline int shm_unlink(const char *name) {
    return ::shm_unlink(name);
}

// prot:
// PROT_READ
// PROT_WRITE
// PROT_EXEC
// PROT_NONE

// flags
// MAP_ANONYMOUS
// MAP_ANON
// MAP_FILE
// MAP_FIXED
// MAP_HASSEMAPHORE
// MAP_PRIVATE
// MAP_SHARED
static inline void *shm_map(void *start_addr, size_t len, int prot, int flags, shm_t fd, off_t offset) {
    return ::mmap(start_addr, len, prot, flags, fd, offset);
}

static inline int shm_unmap(void *addr, size_t len) {
    return ::munmap(addr, len);
}

static inline int shm_stat(shm_t fd, struct stat *st) {
    return ::fstat(fd, st);
}

} // namespace ipc

} // namespace sdk

} // namespace ars
