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
 * @file dir.cpp
 * @brief 
 * @author wotsen (astralrovers@outlook.com)
 * @version 1.0.0
 * @date 2021-04-05
 * 
 * @copyright MIT
 * 
 */
#include "ars/sdk/file/dir.hpp"
#include "ars/sdk/file/file.hpp"
#include <sys/stat.h>
#include <dirent.h>

namespace ars {

namespace sdk {

static bool less(const dir_t& lhs, const dir_t& rhs) {
    return strcasecmp(lhs.name, rhs.name) < 0;
}

int listdir(const char* dir, std::list<dir_t>& dirs) {
    int dirlen = strlen(dir);
    if (dirlen > 256) {
        return -1;
    }
    char path[512];
    strcpy(path, dir);
    if (dir[dirlen-1] != '/') {
        strcat(path, "/");
        ++dirlen;
    }
    dirs.clear();
    // opendir -> readdir -> closedir
    DIR* dp = opendir(dir);
    if (dp == NULL) return -1;
    struct dirent* result = NULL;
    struct stat st;
    dir_t tmp;
    while ((result = readdir(dp))) {
        memset(&tmp, 0, sizeof(dir_t));
        strncpy(tmp.name, result->d_name, sizeof(tmp.name));
        strncpy(path+dirlen, result->d_name, sizeof(path)-dirlen);
        if (lstat(path, &st) == 0) {
            if (S_ISREG(st.st_mode))        tmp.type = 'f';
            else if (S_ISDIR(st.st_mode))   tmp.type = 'd';
            else if (S_ISLNK(st.st_mode))   tmp.type = 'l';
            else if (S_ISBLK(st.st_mode))   tmp.type = 'b';
            else if (S_ISCHR(st.st_mode))   tmp.type = 'c';
            else if (S_ISSOCK(st.st_mode))  tmp.type = 's';
            else if (S_ISFIFO(st.st_mode))  tmp.type = 'p';
            else                            tmp.type = '-';
            tmp.mode = st.st_mode & 0777;
            tmp.size = st.st_size;
            tmp.atime = st.st_atime;
            tmp.mtime = st.st_mtime;
            tmp.ctime = st.st_ctime;
        }
        dirs.push_back(tmp);
    }
    closedir(dp);
    dirs.sort(less);
    return dirs.size();
}

static int dfs_dir_size(const char *path, uint64_t *size)
{
    DIR *pdir = NULL;
    struct dirent *ent = NULL;
    char full_path[ARS_MAX_PATH];
    int ret;
    pdir = opendir(path);
    if (!pdir) {
        // printf("can not open path: %s\n", path);
        if (errno == EMFILE) {
            return -EMFILE;
        } else {
            return -1;
        }
    }
    while (NULL != (ent = readdir(pdir))) {
        memset(full_path, 0, sizeof(full_path));
        sprintf(full_path, "%s/%s", path, ent->d_name);
        if (ent->d_type == DT_DIR) {
            if (!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, "..")) {
                continue;
            }
            ret = dfs_dir_size(full_path, size);
            if (ret == -EMFILE) {
                closedir(pdir);
                return ret;
            }
        } else if (ent->d_type == DT_REG) {
            *size += fs::fs_size(full_path);
        }
    }
    closedir(pdir);
    return 0;
}

int dir_size(const char *path, uint64_t *size)
{
    *size = 0;
    return dfs_dir_size(path, size);
}

int num_in_dir(const char *path)
{
    if (!path) {
        return -1;
    }
    DIR *dir = NULL;
    struct dirent *ent = NULL;
    int num = 0;
    dir = opendir(path);
    if (dir == NULL) {
        return -1;
    }
    while (NULL != (ent = readdir(dir))) {
        if (!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, "..")) {
            continue;
        }
        num++;
    }
    closedir(dir);
    return num;
}

} // namespace sdk

} // namespace ars
