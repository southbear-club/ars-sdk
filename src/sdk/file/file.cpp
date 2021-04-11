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
 * @file file.cpp
 * @brief 
 * @author  ()
 * @version 1.0.0
 * @date 2021-04-07
 * 
 * @copyright MIT
 * 
 */
#include "aru/sdk/file/file.hpp"
#include "aru/sdk/memory/mem.hpp"
#include <assert.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/statfs.h>
#include <sys/vfs.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/param.h>
#include <sys/mount.h>
#include <fcntl.h>

namespace aru {

namespace sdk {

namespace fs {

/*
 * Most of these MAGIC constants are defined in /usr/include/linux/magic.h,
 * and some are hardcoded in kernel sources.
 */
typedef enum fs_type_supported : int64_t {
    FS_CIFS     = 0xFF534D42,
    FS_CRAMFS   = 0x28cd3d45,
    FS_DEBUGFS  = 0x64626720,
    FS_DEVFS    = 0x1373,
    FS_DEVPTS   = 0x1cd1,
    FS_EXT      = 0x137D,
    FS_EXT2_OLD = 0xEF51,
    FS_EXT2     = 0xEF53,
    FS_EXT3     = 0xEF53,
    FS_EXT4     = 0xEF53,
    FS_FUSE     = 0x65735546,
    FS_JFFS2    = 0x72b6,
    FS_MQUEUE   = 0x19800202,
    FS_MSDOS    = 0x4d44,
    FS_NFS      = 0x6969,
    FS_NTFS     = 0x5346544e,
    FS_PROC     = 0x9fa0,
    FS_RAMFS    = 0x858458f6,
    FS_ROMFS    = 0x7275,
    FS_SELINUX  = 0xf97cff8c,
    FS_SMB      = 0x517B,
    FS_SOCKFS   = 0x534F434B,
    FS_SQUASHFS = 0x73717368,
    FS_SYSFS    = 0x62656572,
    FS_TMPFS    = 0x01021994
} fs_type_supported_t;

static struct {
    const char name[32];
    const int64_t value;
} fs_type_info[] = {
    {"CIFS    ", FS_CIFS    },
    {"CRAMFS  ", FS_CRAMFS  },
    {"DEBUGFS ", FS_DEBUGFS },
    {"DEVFS   ", FS_DEVFS   },
    {"DEVPTS  ", FS_DEVPTS  },
    {"EXT     ", FS_EXT     },
    {"EXT2_OLD", FS_EXT2_OLD},
    {"EXT2    ", FS_EXT2    },
    {"EXT3    ", FS_EXT3    },
    {"EXT4    ", FS_EXT4    },
    {"FUSE    ", FS_FUSE    },
    {"JFFS2   ", FS_JFFS2   },
    {"MQUEUE  ", FS_MQUEUE  },
    {"MSDOS   ", FS_MSDOS   },
    {"NFS     ", FS_NFS     },
    {"NTFS    ", FS_NTFS    },
    {"PROC    ", FS_PROC    },
    {"RAMFS   ", FS_RAMFS   },
    {"ROMFS   ", FS_ROMFS   },
    {"SELINUX ", FS_SELINUX },
    {"SMB     ", FS_SMB     },
    {"SOCKFS  ", FS_SOCKFS  },
    {"SQUASHFS", FS_SQUASHFS},
    {"SYSFS   ", FS_SYSFS   },
    {"TMPFS   ", FS_TMPFS   },
};

extern const struct file_ops io_ops;
extern const struct file_ops fio_ops;

static const struct file_ops *file_ops[] = {
    &io_ops,
    &fio_ops,
    NULL
};

static file_backend_type backend = FILE_BACKEND_IO;

bool fs_exists(const char* path) {
    struct stat attr;
    return ::lstat(path, &attr) == 0;
}

bool fs_isdir(const char* path) {
    struct stat attr;
    if (::lstat(path, &attr) != 0) return false;
    return S_ISDIR(attr.st_mode);
}

int64_t fs_mtime(const char* path) {
    struct stat attr;
    if (::lstat(path, &attr) != 0) return -1;
    return attr.st_mtime;
}

int64_t fs_size(const char* path) {
    struct stat attr;
    if (::lstat(path, &attr) != 0) return -1;
    return attr.st_size;
}

bool fs_create(const char *path) {
    struct file *fp = file_open(path, F_CREATE);
    if (!fp) {
        return -1;
    }
    file_close(fp);
    return 0;
}

// rf = false  ->  rm or rmdir
// rf = true   ->  rm -rf
bool fs_remove(const char* path, bool rf) {
    if (!fs_exists(path)) return true;

    if (!rf) {
        if (fs_isdir(path)) return ::rmdir(path) == 0;
        return ::remove(path) == 0;
    } else {
        std::string cmd("rm -rf \"");
        cmd += path;
        cmd += "\"";
        // FIXME:替换系统调用
        return system(cmd.c_str()) != -1;
    }
}

bool fs_rename(const char* from, const char* to) {
    return ::rename(from, to) == 0;
}

bool symlink(const char* dst, const char* lnk) {
    remove(lnk);
    return ::symlink(dst, lnk) == 0;
}

struct file *file_open(const char *path, file_open_mode_t mode)
{
    struct file *file = (struct file *)aru_calloc(1, sizeof(struct file));
    if (!file) {
        // printf("malloc failed!\n");
        return nullptr;
    }
    file->ops = file_ops[backend];
    file->fd = file->ops->open(path, mode);
    return file;
}

void file_close(struct file *file)
{
    if (!file || !file->ops) {
        return;
    }
    file->ops->close(file->fd);
    aru_free(file);
}

ssize_t file_read(struct file *file, void *data, size_t size)
{
    if (!file || !data || size == 0) {
        return -1;
    }
    return file->ops->read(file->fd, data, size);
}

ssize_t file_read_path(const char *path, void *data, size_t size)
{
    ssize_t flen = 0;
    struct file *fp = file_open(path, F_RDONLY);
    if (!fp) {
        return -1;
    }
    flen = file_read(fp, data, size);
    file_close(fp);
    return flen;
}

ssize_t file_write(struct file *file, const void *data, size_t size)
{
    if (!file || !data || size == 0) {
        return -1;
    }
    return file->ops->write(file->fd, data, size);
}

ssize_t file_write_path(const char *path, const void *data, size_t size)
{
    ssize_t flen = 0;
    struct file *fp = file_open(path, F_WRCLEAR);
    if (!fp) {
        return -1;
    }
    flen = file_write(fp, data, size);
    file_close(fp);
    return flen;
}

ssize_t file_size(struct file *file)
{
    if (!file) {
        return -1;
    }
    return file->ops->size(file->fd);
}

int file_sync(struct file *file)
{
    if (!file) {
        return -1;
    }
    return file->ops->sync(file->fd);
}

off_t file_seek(struct file *file, off_t offset, int whence)
{
    if (!file) {
        return -1;
    }
    return file->ops->seek(file->fd, offset, whence);
}

struct iovec *file_dump(const char *path)
{
    struct iovec *buf = NULL;
    struct file *f = NULL;
    ssize_t size = 0;
    if (!path) {
        return NULL;
    }
    size = fs_size(path);
    if (size == 0) {
        return NULL;
    }
    buf = (struct iovec *)aru_calloc(1, sizeof(struct iovec));
    if (!buf) {
        // printf("malloc failed!\n");
        return NULL;
    }
    buf->iov_len = size;
    buf->iov_base = aru_calloc(1, buf->iov_len);
    if (!buf->iov_base) {
        // printf("malloc failed!\n");
        return NULL;
    }

    f = file_open(path, F_RDONLY);
    if (!f) {
        // printf("file open failed!\n");
        aru_free(buf->iov_base);
        aru_free(buf);
        return NULL;
    }
    file_read(f, buf->iov_base, buf->iov_len);
    file_close(f);
    return buf;
}

int file_get_systat(const char *path, struct file_systat *fi) {
    int i;
    struct statfs stfs;
    if (!path || !fi) {
        // printf("path can't be null\n");
        return -1;
    }
    if (-1 == ::statfs(path, &stfs)) {
        // printf("statfs %s failed: %s\n", path, strerror(errno));
        return -1;
    }
    fi->size_total = stfs.f_bsize * stfs.f_blocks;
    fi->size_avail = stfs.f_bsize * stfs.f_bavail;
    fi->size_free  = stfs.f_bsize * stfs.f_bfree;
    for (i = 0; i < (int)ARU_ARRAY_SIZE(fs_type_info); i++) {
        if (stfs.f_type == fs_type_info[i].value) {
            stfs.f_type = i;
            strncpy(fi->fs_type_name, fs_type_info[i].name,
                            sizeof(fi->fs_type_name));
            break;
        }
    }
    return 0;
}

int file_get_info(const char *path, struct file_info *info)
{
    struct stat st;
    if (-1 == stat(path, &st)) {
        // printf("stat %s failed!\n", path);
        return -1;
    }
    switch (st.st_mode & S_IFMT) {
    case S_IFSOCK:
        info->type = F_SOCKET;
        break;
    case S_IFLNK:
        info->type = F_LINK;
        break;
    case S_IFBLK:
    case S_IFCHR:
        info->type = F_DEVICE;
        break;
    case S_IFREG:
        info->type = F_NORMAL;
        break;
    case S_IFDIR:
        info->type = F_DIR;
        break;
    default:
        break;
    }
    info->size = st.st_size;
    info->access_sec = st.st_atim.tv_sec;
    info->modify_sec = st.st_ctim.tv_sec;//using change, not modify
    return 0;
}

} // namespace fs

} // namespace sdk

} // namespace aru
