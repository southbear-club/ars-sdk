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
 * @file fio.cpp
 * @brief 
 * @author  ()
 * @version 1.0.0
 * @date 2021-04-10
 * 
 * @copyright MIT
 * 
 */
#include "aru/sdk/file/file.hpp"
#include "aru/sdk/memory/mem.hpp"
#include <assert.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>

namespace aru {
    
namespace sdk {
    
namespace fs {

#define MAX_RETRY_CNT   (3)

static struct file_desc *fio_open(const char *path, file_open_mode_t mode)
{
    const char *flags = NULL;
    struct file_desc *file = (struct file_desc *)aru_calloc(1,
                             sizeof(struct file_desc));
    if (!file) {
        // printf("malloc failed:%d %s\n", errno, strerror(errno));
        return NULL;
    }
    switch(mode) {
    case F_RDONLY:
        flags = "r";
        break;
    case F_WRONLY:
        flags = "w";
        break;
    case F_RDWR:
        flags = "r+";
        break;
    case F_CREATE:
        flags = "w+";
        break;
    case F_WRCLEAR:
        flags = "w+";
        break;
    case F_APPEND:
        flags = "a+";
        break;
    default:
        // printf("unsupport file mode!\n");
        break;
    }

    file->fp = fopen(path, flags);
    if (!file->fp) {
        // printf("fopen %s failed:%d %s\n", path, errno, strerror(errno));
        aru_free(file);
        return NULL;
    }
    file->name = strdup(path);
    return file;
}

static ssize_t fio_read(struct file_desc *file, void *buf, size_t len)
{
    int n;
    FILE *fp = NULL;
    char *p = (char *)buf;
    size_t left = len;
    size_t step = 1024*1024;
    int cnt = 0;
    if (file == NULL || buf == NULL || len == 0) {
        // printf("%s paraments invalid!\n", __func__);
        return -1;
    }
    fp = file->fp;
    while (left > 0) {
        if (left < step)
            step = left;
        n = fread((void *)p, 1, step, fp);
        if (n > 0) {
            p += n;
            left -= n;
            continue;
        } else {
            if (0 != feof(fp)) {
                clearerr(fp);
                break;
            } else {
                if (++cnt > MAX_RETRY_CNT)
                    break;
                continue;
            }
        }
    }
    return (len - left);
}

static ssize_t fio_write(struct file_desc *file, const void *buf, size_t len)
{
    FILE *fp = NULL;
    ssize_t n;
    size_t step;
    size_t left;
    char *p = (char *)buf;
    int retry = 0;
    if (file == NULL || buf == NULL || len == 0) {
        // printf("%s paraments invalid!\n", __func__);
        return -1;
    }
    fp = file->fp;
    step = 1024 * 1024;
    left = len;
    while (left > 0) {
        if (left < step)
            step = left;
        n = fwrite((void *)p, 1, step, fp);
        if (n > 0) {
            p += n;
            left -= n;
            continue;
        } else {
            if (errno == EINTR || errno == EAGAIN) {
                if (++retry > MAX_RETRY_CNT) {
                    // printf("reach max retry\n");
                    break;
                }
                continue;
            } else {
                // printf("write failed:%d %s\n", errno, strerror(errno));
                break;
            }
        }
    }
    return (len - left);
}

static off_t fio_seek(struct file_desc *file, off_t offset, int whence)
{
    if (!file) {
        return -1;
    }
    return fseek(file->fp, offset, whence);
}

static int fio_sync(struct file_desc *file)
{
    if (!file) {
        return -1;
    }
    return fflush(file->fp);
}

static size_t fio_size(struct file_desc *file)
{
    long size;
    long tmp;
    if (!file) {
        return 0;
    }
    tmp = ftell(file->fp);
    fseek(file->fp, 0L, SEEK_END);
    size = ftell(file->fp);
    fseek(file->fp, tmp, SEEK_SET);
    return (size_t)size;
}

static void fio_close(struct file_desc *file)
{
    if (file) {
        fclose(file->fp);
        aru_free(file->name);
        aru_free(file);
    }
}

struct file_ops fio_ops = {
    fio_open,
    fio_write,
    fio_read,
    fio_seek,
    fio_sync,
    fio_size,
    fio_close,
};

} // namespace fs

} // namespace sdk

} // namespace aru
