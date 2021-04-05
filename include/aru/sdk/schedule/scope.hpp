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
 * @file scope.hpp
 * @brief 
 * @author wotsen (astralrovers@outlook.com)
 * @version 1.0.0
 * @date 2021-04-05
 * 
 * @copyright MIT
 * 
 */
#pragma once
#include <functional>
#include "../macros/defs.hpp"

namespace aru {

namespace sdk {

typedef std::function<void()> Function;

// same as golang defer
class Defer {
public:
    Defer(Function&& fn) : _fn(std::move(fn)) {}
    ~Defer() { if(_fn) _fn();}
private:
    Function _fn;
};
#define defer(code) Defer ARU_STRINGCAT(_defer_, __LINE__)([&](){code});

class ScopeCleanup {
public:
    template<typename Fn, typename... Args>
    ScopeCleanup(Fn&& fn, Args&&... args) {
        _cleanup = std::bind(std::forward<Fn>(fn), std::forward<Args>(args)...);
    }

    ~ScopeCleanup() {
        _cleanup();
    }

private:
    Function _cleanup;
};

template<typename T>
class ScopeFree {
public:
    ScopeFree(T* p) : _p(p) {}
    ~ScopeFree()    {ARU_SAFE_FREE(_p);}
private:
    T*  _p;
};

template<typename T>
class ScopeDelete {
public:
    ScopeDelete(T* p) : _p(p) {}
    ~ScopeDelete()    {ARU_SAFE_DELETE(_p);}
private:
    T*  _p;
};

template<typename T>
class ScopeDeleteArray {
public:
    ScopeDeleteArray(T* p) : _p(p) {}
    ~ScopeDeleteArray()    {ARU_SAFE_DELETE_ARRAY(_p);}
private:
    T*  _p;
};

template<typename T>
class ScopeRelease {
public:
    ScopeRelease(T* p) : _p(p) {}
    ~ScopeRelease()    {ARU_SAFE_RELEASE(_p);}
private:
    T*  _p;
};

template<typename T>
class ScopeLock {
public:
    ScopeLock(T& mutex) : _mutex(mutex) {_mutex.lock();}
    ~ScopeLock()    {_mutex.unlock();}
private:
    T& _mutex;
};

} // namespace sdk

} // namespace aru
