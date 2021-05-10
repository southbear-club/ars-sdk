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
 * @file file_obj.hpp
 * @brief 
 * @author  ()
 * @version 1.0.0
 * @date 2021-04-10
 * 
 * @copyright MIT
 * 
 */
#pragma once

#include <string.h>
#include <unistd.h>
#include <string>
#include <sys/stat.h>
#include "path.hpp"
#include "../ds/buf.hpp"
#include "../macros/defs.hpp"

namespace ars {

namespace sdk {

/// 文件对象
class File {
public:
    File() {
        fp = NULL;
    }

    ~File() {
        close();
    }

    int open(const char* filepath, const char* mode) {
        close();
        memcpy(this->filepath, filepath, strlen(filepath) < ARS_MAX_PATH ? strlen(filepath) : ARS_MAX_PATH);
        fp = fopen(filepath, mode);
        return fp ? 0 : errno;
    }

    void close() {
        if (fp) {
            fclose(fp);
            fp = NULL;
        }
    }

    size_t read(void* ptr, size_t len) {
        return fread(ptr, 1, len, fp);
    }

    size_t write(const void* ptr, size_t len) {
        return fwrite(ptr, 1, len, fp);
    }

    size_t size() {
        struct stat st;
        memset(&st, 0, sizeof(st));
        stat(filepath, &st);
        return st.st_size;
    }

    size_t readall(Buf& buf) {
        size_t filesize = size();
        buf.resize(filesize);
        return fread(buf.base, 1, filesize, fp);
    }

    size_t readall(std::string& str) {
        size_t filesize = size();
        str.resize(filesize);
        return fread((void*)str.data(), 1, filesize, fp);
    }

    bool readline(std::string& str) {
        str.clear();
        char ch;
        while (fread(&ch, 1, 1, fp)) {
            if (ch == ARS_LF) {
                // unix: LF
                return true;
            }
            if (ch == ARS_CR) {
                // dos: CRLF
                // read LF
                if (fread(&ch, 1, 1, fp) && ch != ARS_LF) {
                    // mac: CR
                    fseek(fp, -1, SEEK_CUR);
                }
                return true;
            }
            str += ch;
        }
        return str.length() != 0;
    }

    int readrange(std::string& str, size_t from = 0, size_t to = 0) {
        size_t filesize = size();
        if (to == 0 || to >= filesize) to = filesize - 1;
        size_t readbytes = to - from + 1;
        str.resize(readbytes);
        fseek(fp, from, SEEK_SET);
        return fread((void*)str.data(), 1, readbytes, fp);
    }

public:
    char  filepath[ARS_MAX_PATH];
    std::string filepath_;
    FILE* fp;
};

}
}