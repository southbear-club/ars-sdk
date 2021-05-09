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
 * @file scope_guard.hpp
 * @brief 
 * @author wotsen (astralrovers@outlook.com)
 * @version 1.0.0
 * @date 2021-05-05
 * 
 * @copyright MIT
 * 
 */
#pragma once
#include "noncopyable.hpp"
#include "unused.hpp"
#include "make.hpp"
#include <utility>      // std::forward, std::move
#include <algorithm>    // std::swap
#include <functional>   // std::function

namespace aru {
    
namespace sdk {

////////////////////////////////////////////////////////////////
/// Execute guard function when the enclosing scope exits
////////////////////////////////////////////////////////////////

template <typename F = std::function<void()>>
class scope_guard : aru::sdk::noncopyable
{
    F destructor_;
    mutable bool dismiss_;

public:
    template <typename F_>
    scope_guard(F_&& destructor)
        : destructor_(std::forward<F_>(destructor))
        , dismiss_(false)
    {}

    scope_guard(scope_guard&& rhs)
        : destructor_(std::move(rhs.destructor_))
        , dismiss_(true) // dismiss rhs
    {
        std::swap(dismiss_, rhs.dismiss_);
    }

    ~scope_guard(void)
    {
        try { do_exit(); }
        /*
            In the realm of exceptions, it is fundamental that you can do nothing
            if your "undo/recover" action fails.
        */
        catch(...) { /* Do nothing */ }
    }

    void dismiss(void) const noexcept
    {
        dismiss_ = true;
    }

    void do_exit(void)
    {
        if (!dismiss_)
        {
            dismiss_ = true;
            destructor_();
        }
    }

    void swap(scope_guard& rhs)
    {
        std::swap(destructor_, rhs.destructor_);
        std::swap(dismiss_,    rhs.dismiss_);
    }
};

struct helper
{
    template <typename F>
    auto operator=(F&& destructor) -> decltype(aru::sdk::make<scope_guard>(std::forward<F>(destructor)))
    {
        return aru::sdk::make<scope_guard>(std::forward<F>(destructor));
    }
};

#define ARU_SCOPE_GUARD_V_(L)  ARU_UNUSED_ scope_guard_##L##__
#define ARU_SCOPE_GUARD_L_(L)  auto ARU_SCOPE_GUARD_V_(L) = aru::sdk::helper{}

/*
    Do things like this:
    -->
    ARU_SCOPE_GUARD_ = [ptr]
    {
        if (ptr) free(ptr);
    };
*/

#define ARU_SCOPE_GUARD_ ARU_SCOPE_GUARD_L_(__LINE__)

} // namespace sdk

} // namespace aru
