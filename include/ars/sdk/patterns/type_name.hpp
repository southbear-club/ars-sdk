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
 * @file type_name.hpp
 * @brief 
 * @author wotsen (astralrovers@outlook.com)
 * @version 1.0.0
 * @date 2021-05-05
 * 
 * @copyright MIT
 * 
 */
#pragma once
#include "scope_guard.hpp"
#include <typeinfo>     // typeid
#include <sstream>      // std::ostringstream, std::string
#include <type_traits>  // std::is_array

#if defined(__GNUC__)
#   include <cxxabi.h>  // abi::__cxa_demangle
#endif/*__GNUC__*/

namespace ars {
    
namespace sdk {

////////////////////////////////////////////////////////////////
/// Ready for output
////////////////////////////////////////////////////////////////

// Forward declarations
template <typename T, bool IsBase = false>
struct check;

/*
    Output state management
*/

class output
{
    bool is_compact_ = true;

    template <typename T>
    bool check_empty(const T&) { return false; }
    bool check_empty(const char* val)
    {
        return (!val) || (val[0] == 0);
    }

    template <typename T>
    void out(const T& val)
    {
        if (check_empty(val)) return;
        if (!is_compact_) sr_ += " ";
        std::ostringstream ss;
        ss << val;
        sr_ += ss.str();
        is_compact_ = false;
    }

    std::string& sr_;

public:
    output(std::string& sr) : sr_(sr) {}

    output& operator()(void) { return (*this); }

    template <typename T1, typename... T>
    output& operator()(const T1& val, const T&... args)
    {
        out(val);
        return operator()(args...);
    }

    output& compact(void)
    {
        is_compact_ = true;
        return (*this);
    }
};

// ()

template <bool>
struct bracket
{
    output& out_;

    bracket(output& out, const char* = nullptr) : out_(out)
    { out_("(").compact(); }

    ~bracket(void)
    { out_.compact()(")"); }
};

template <>
struct bracket<false>
{
    bracket(output& out, const char* str = nullptr)
    { out(str); }
};

// [N]

template <size_t N = 0>
struct bound
{
    output& out_;

    bound(output& out) : out_(out) {}
    ~bound(void)
    {
        if (N == 0) out_("[]");
        else        out_("[").compact()
                        ( N ).compact()
                        ("]");
    }
};

// (P1, P2, ...)

template <bool, typename... P>
struct parameter;

template <bool IsStart>
struct parameter<IsStart>
{
    output& out_;

    parameter(output& out) : out_(out) {}
    ~parameter(void)
    { bracket<IsStart> { out_ }; }
};

template <bool IsStart, typename P1, typename... P>
struct parameter<IsStart, P1, P...>
{
    output& out_;

    parameter(output& out) : out_(out) {}
    ~parameter(void)
    {
        [this](bracket<IsStart>&&)
        {
            check<P1> { out_ };
            parameter<false, P...> { out_.compact() };
        } (bracket<IsStart> { out_, "," });
    }
};

////////////////////////////////////////////////////////////////
/// Template specializations for checking
////////////////////////////////////////////////////////////////

/*
    CV-qualifiers, references, pointers
*/

template <typename T, bool IsBase>
struct check
{
    output out_;
    check(const output& out) : out_(out)
    {
#   if defined(__GNUC__)
        const char* typeid_name = typeid(T).name();
        char* real_name = abi::__cxa_demangle(typeid_name, nullptr, nullptr, nullptr);
        ARS_SCOPE_GUARD_ = [real_name]
        {
            if (real_name) ::free(real_name);
        };
        out_(real_name ? real_name : typeid_name);
#   else /*__GNUC__*/
        out_(typeid(T).name());
#   endif/*__GNUC__*/
    }
};

#pragma push_macro("ARS_CHECK_TYPE__")
#undef  ARS_CHECK_TYPE__
#define ARS_CHECK_TYPE__(OPT)                                 \
    template <typename T, bool IsBase>                         \
    struct check<T OPT, IsBase> : check<T, true>               \
    {                                                          \
        using base_t = check<T, true>;                         \
        using base_t::out_;                                    \
        check(const output& out) : base_t(out) { out_(#OPT); } \
    };

ARS_CHECK_TYPE__(const)
ARS_CHECK_TYPE__(volatile)
ARS_CHECK_TYPE__(const volatile)
ARS_CHECK_TYPE__(&)
ARS_CHECK_TYPE__(&&)
ARS_CHECK_TYPE__(*)

#pragma pop_macro("ARS_CHECK_TYPE__")

/*
    Arrays
*/

#pragma push_macro("ARS_CHECK_TYPE_ARRAY__")
#pragma push_macro("ARS_CHECK_TYPE_ARRAY_CV__")
#pragma push_macro("ARS_CHECK_TYPE_PLACEHOLDER__")

#undef ARS_CHECK_TYPE_ARRAY__
#undef ARS_CHECK_TYPE_ARRAY_CV__
#undef ARS_CHECK_TYPE_PLACEHOLDER__

#define ARS_CHECK_TYPE_ARRAY__(CV_OPT, BOUND_OPT, ...)                                    \
    template <typename T, bool IsBase __VA_ARGS__>                                         \
    struct check<T CV_OPT [BOUND_OPT], IsBase> : check<T CV_OPT, !std::is_array<T>::value> \
    {                                                                                      \
        using base_t = check<T CV_OPT, !std::is_array<T>::value>;                          \
        using base_t::out_;                                                                \
                                                                                           \
        bound<BOUND_OPT> bound_   = out_;                                                  \
        bracket<IsBase>  bracket_ = out_;                                                  \
                                                                                           \
        check(const output& out) : base_t(out) {}                                          \
    };

#define ARS_CHECK_TYPE_ARRAY_CV__(BOUND_OPT, ...)               \
    ARS_CHECK_TYPE_ARRAY__(, BOUND_OPT, ,##__VA_ARGS__)         \
    ARS_CHECK_TYPE_ARRAY__(const, BOUND_OPT, ,##__VA_ARGS__)    \
    ARS_CHECK_TYPE_ARRAY__(volatile, BOUND_OPT, ,##__VA_ARGS__) \
    ARS_CHECK_TYPE_ARRAY__(const volatile, BOUND_OPT, ,##__VA_ARGS__)

#define ARS_CHECK_TYPE_PLACEHOLDER__

ARS_CHECK_TYPE_ARRAY_CV__(ARS_CHECK_TYPE_PLACEHOLDER__)
#if defined(__GNUC__)
ARS_CHECK_TYPE_ARRAY_CV__(0)
#endif/*__GNUC__*/
ARS_CHECK_TYPE_ARRAY_CV__(N, size_t N)

#pragma pop_macro("ARS_CHECK_TYPE_PLACEHOLDER__")
#pragma pop_macro("ARS_CHECK_TYPE_ARRAY_CV__")
#pragma pop_macro("ARS_CHECK_TYPE_ARRAY__")

/*
    Functions
*/

template <typename T, bool IsBase, typename... P>
struct check<T(P...), IsBase> : check<T, true>
{
    using base_t = check<T, true>;
    using base_t::out_;

    parameter<true, P...> parameter_ = out_;
    bracket<IsBase>       bracket_   = out_;

    check(const output& out) : base_t(out) {}
};

/*
    Pointers to members
*/

template <typename T, bool IsBase, typename C>
struct check<T C::*, IsBase> : check<T, true>
{
    using base_t = check<T, true>;
    using base_t::out_;

    check(const output& out) : base_t(out)
    {
        check<C> { out_ };
        out_.compact()("::*");
    }
};

/*
    Pointers to member functions
*/

#pragma push_macro("ARS_CHECK_TYPE_MEM_FUNC__")
#undef  ARS_CHECK_TYPE_MEM_FUNC__
#define ARS_CHECK_TYPE_MEM_FUNC__(...)                           \
    template <typename T, bool IsBase, typename C, typename... P> \
    struct check<T(C::*)(P...) __VA_ARGS__, IsBase>               \
    {                                                             \
        scope_guard<> at_destruct_cv_;                            \
        check<T(P...), true> base_;                               \
        output& out_ = base_.out_;                                \
                                                                  \
        check(const output& out)                                  \
            : at_destruct_cv_([this]{ out_(#__VA_ARGS__); })      \
            , base_(out)                                          \
        {                                                         \
            check<C> { out_ };                                    \
            out_.compact()("::*");                                \
        }                                                         \
    };

ARS_CHECK_TYPE_MEM_FUNC__()
ARS_CHECK_TYPE_MEM_FUNC__(const)
ARS_CHECK_TYPE_MEM_FUNC__(volatile)
ARS_CHECK_TYPE_MEM_FUNC__(const volatile)

#pragma pop_macro("ARS_CHECK_TYPE_MEM_FUNC__")

/*
    type_name<const volatile void *>()
    -->
    void const volatile *
*/

template <typename T>
inline std::string type_name(void)
{
    std::string str;
    ars::sdk::check<T> { str };
    return str;
}

} // namespace sdk

} // namespace ars
