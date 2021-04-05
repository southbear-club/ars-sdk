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
 * @file buf.hpp
 * @brief 
 * @author wotsen (astralrovers@outlook.com)
 * @version 1.0.0
 * @date 2021-04-04
 * 
 * @copyright MIT
 * 
 */
#pragma once
#include <stddef.h>
#include <string.h>
#include <algorithm>
#include "../memory/mem.hpp"

namespace aru {

namespace sdk {

typedef struct buf_s {
    char*  base;
    size_t len;

    buf_s() {
        base = NULL;
        len  = 0;
    }

    buf_s(void* data, size_t len) {
        this->base = (char*)data;
        this->len  = len;
    }
} buf_t;

typedef struct offset_buf_s {
    char*   base;
    size_t  len;
    size_t  offset;
    offset_buf_s() {
        base = NULL;
        len = offset = 0;
    }

    offset_buf_s(void* data, size_t len) {
        this->base = (char*)data;
        this->len = len;
    }
} offset_buf_t;

class Buf : public buf_t {
public:
    Buf() : buf_t() {
        cleanup_ = false;
    }
    Buf(void* data, size_t len) : buf_t(data, len) {
        cleanup_ = false;
    }
    Buf(size_t cap) { resize(cap); }

    virtual ~Buf() {
        cleanup();
    }

    void*  data() { return base; }
    size_t size() { return len; }

    bool isNull() { return base == NULL || len == 0; }

    void cleanup() {
        if (cleanup_) {
            ARU_FREE(base);
            len = 0;
            cleanup_ = false;
        }
    }

    void resize(size_t cap) {
        if (cap == len) return;

        if (base == NULL) {
            ARU_ALLOC(base, cap);
        }
        else {
            base = (char*)aru::sdk::aru_realloc(base, cap, len);
        }
        len = cap;
        cleanup_ = true;
    }

    void copy(void* data, size_t len) {
        resize(len);
        memcpy(base, data, len);
    }

    void copy(buf_t* buf) {
        copy(buf->base, buf->len);
    }

private:
    bool cleanup_;
};

// VL: Variable-Length
class VLBuf : public Buf {
public:
    VLBuf() : Buf() {_offset = _size = 0;}
    VLBuf(void* data, size_t len) : Buf(data, len) {_offset = 0; _size = len;}
    VLBuf(size_t cap) : Buf(cap) {_offset = _size = 0;}
    virtual ~VLBuf() {}

    char* data() { return base + _offset; }
    size_t size() { return _size; }

    void push_front(void* ptr, size_t len) {
        if (len > this->len - _size) {
            size_t newsize = std::max(this->len, len)*2;
            base = (char*)aru::sdk::aru_realloc(base, newsize, this->len);
            this->len = newsize;
        }

        if (_offset < len) {
            // move => end
            memmove(base+this->len-_size, data(), _size);
            _offset = this->len-_size;
        }

        memcpy(data()-len, ptr, len);
        _offset -= len;
        _size += len;
    }

    void push_back(void* ptr, size_t len) {
        if (len > this->len - _size) {
            size_t newsize = std::max(this->len, len)*2;
            base = (char*)aru::sdk::aru_realloc(base, newsize, this->len);
            this->len = newsize;
        }
        else if (len > this->len - _offset - _size) {
            // move => start
            memmove(base, data(), _size);
            _offset = 0;
        }
        memcpy(data()+_size, ptr, len);
        _size += len;
    }

    void pop_front(void* ptr, size_t len) {
        if (len <= _size) {
            if (ptr) {
                memcpy(ptr, data(), len);
            }
            _offset += len;
            if (_offset >= len) _offset = 0;
            _size   -= len;
        }
    }

    void pop_back(void* ptr, size_t len) {
        if (len <= _size) {
            if (ptr) {
                memcpy(ptr, data()+_size-len, len);
            }
            _size -= len;
        }
    }

    void clear() {
        _offset = _size = 0;
    }

    void prepend(void* ptr, size_t len) {
        push_front(ptr, len);
    }

    void append(void* ptr, size_t len) {
        push_back(ptr, len);
    }

    void insert(void* ptr, size_t len) {
        push_back(ptr, len);
    }

    void remove(size_t len) {
        pop_front(NULL, len);
    }

private:
    size_t _offset;
    size_t _size;
};

// 循环缓冲区
class RingBuf : public Buf {
public:
    RingBuf() : Buf() {_head = _tail = _size = 0;}
    RingBuf(size_t cap) : Buf(cap) {_head = _tail = _size = 0;}
    virtual ~RingBuf() {}

    char* alloc(size_t len) {
        char* ret = NULL;
        if (_head < _tail || _size == 0) {
            // [_tail, this->len) && [0, _head)
            if (this->len - _tail >= len) {
                ret = base + _tail;
                _tail += len;
                if (_tail == this->len) _tail = 0;
            }
            else if (_head >= len) {
                ret = base;
                _tail = len;
            }
        }
        else {
            // [_tail, _head)
            if (_head - _tail >= len) {
                ret = base + _tail;
                _tail += len;
            }
        }
        _size += ret ? len : 0;
        return ret;
    }

    void free(size_t len) {
        _size -= len;
        if (len <= this->len - _head) {
            _head += len;
            if (_head == this->len) _head = 0;
        }
        else {
            _head = len;
        }
    }

    void clear() {_head = _tail = _size = 0;}

    size_t size() {return _size;}

private:
    size_t _head;
    size_t _tail;
    size_t _size;
};

} // namespace sdk

} // namespace aru
