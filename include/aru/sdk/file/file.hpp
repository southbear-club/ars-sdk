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
 * @file file.hpp
 * @brief 
 * @author wotsen (astralrovers@outlook.com)
 * @version 1.0.0
 * @date 2021-04-04
 * 
 * @copyright MIT
 * 
 */
#pragma once

#include <string.h>
#include <unistd.h>
#include <string>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/uio.h>
#include "path.hpp"
#include "../macros/defs.hpp"

namespace aru {

namespace sdk {

namespace fs {

typedef enum file_open_mode {
    F_RDONLY,
    F_WRONLY,
    F_RDWR,
    F_CREATE,
    F_WRCLEAR,
    F_APPEND,
} file_open_mode_t;

typedef enum file_type {
    F_NORMAL,
    F_DIR,
    F_LINK,
    F_SOCKET,
    F_DEVICE,

} file_type_t;

typedef struct file_desc {
    union {
        int fd;
        FILE *fp;
    };
    char *name;
} file_desc_t;

typedef struct file_info {
    uint64_t modify_sec;
    uint64_t access_sec;
    enum file_type type;
    char path[ARU_MAX_PATH];
    uint64_t size;
} file_info_t;

typedef struct file_ops {
    struct file_desc * (*open)(const char *path, file_open_mode_t mode);
    ssize_t (*write)(struct file_desc *fd, const void *buf, size_t count);
    ssize_t (*read)(struct file_desc *fd, void *buf, size_t count);
    off_t (*seek)(struct file_desc *fd, off_t offset, int whence);
    int (*sync)(struct file_desc *fd);
    size_t (*size)(struct file_desc *fd);
    void (*close)(struct file_desc *fd);
} file_ops_t;

typedef struct file_systat {
    uint64_t size_total;
    uint64_t size_avail;
    uint64_t size_free;
    char fs_type_name[32];
} file_systat;

typedef struct file {
    struct file_desc *fd;
    const file_ops_t *ops;
    file_info_t info;
} file_t;

typedef enum file_backend_type {
    FILE_BACKEND_IO,
    FILE_BACKEND_FIO,
} file_backend_type;

bool fs_exists(const char* path);
// file size
int64_t fs_size(const char* path);
bool fs_create(const char *path);
// rf = false  ->  rm or rmdir
// rf = true   ->  rm -rf
bool fs_remove(const char* path, bool rf=false);
bool fs_rename(const char* from, const char* to);
// administrator privileges required on windows
bool fs_symlink(const char* dst, const char* lnk);
bool fs_isdir(const char* path);
// modify time
int64_t fs_mtime(const char* path);

void file_backend(file_backend_type type);
struct file *file_open(const char *path, file_open_mode_t mode);
void file_close(struct file *file);
ssize_t file_read(struct file *file, void *data, size_t size);
ssize_t file_read_path(const char *path, void *data, size_t size);
ssize_t file_write(struct file *file, const void *data, size_t size);
ssize_t file_write_path(const char *path, const void *data, size_t size);
ssize_t file_size(struct file *file);
int file_get_info(const char *path, struct file_info *info);
struct iovec *file_dump(const char *path);
int file_sync(struct file *file);
off_t file_seek(struct file *file, off_t offset, int whence);
int file_get_systat(const char *path, struct file_systat *stat);

} // namespace fs

} // namespace sdk

} // namespace aru
