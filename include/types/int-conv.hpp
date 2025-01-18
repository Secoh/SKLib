// This file is part of SKLib: https://github.com/Secoh/SKLib
// Copyright [2020-2025] Secoh
//
// Licensed under the GNU Lesser General Public License, Version 2.1 or later. See: https://www.gnu.org/licenses/
// You may not use this file except in compliance with the License.
// Software is distributed on "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// Special exception from GNU LGPL terms: you don't have to publish the compiled object binary file(s) for SKLib.
// Modified source code and/or any derivative work requirements are still in effect. All such file(s) must be openly
// published under the same terms as the original one(s), but you don't have to inherit the special exception above.

#ifndef SKLIB_INCLUDED_TYPES_INTCONV_HPP
#define SKLIB_INCLUDED_TYPES_INTCONV_HPP

#include <type_traits>

#include "detect-type.hpp"

namespace sklib {

// Provides constants, macros, and templates for detection of C/C++ types.
// This is internal SKLib file and must NOT be included directly.

namespace priv
{
    // Selective materialization of template class depending on arbitrary condition
    // using Partial specialization and SFINAE
    // see: https://stackoverflow.com/questions/72197242/what-is-wrong-with-my-application-of-sfinae-when-trying-to-implement-a-type-trai

    template<class T, typename = void>
    struct make_unsigned_if_int
    {
        using type = T;
        static constexpr auto conv(const T& v) const { return v; }
    };

    template<class T, typename = void>
    struct make_signed_if_int
    {
        using type = T;
    };

    template<class T>
    struct make_unsigned_if_int<T, std::enable_if_t<is_native_integer_v<T>>>
    {
        using type = typename std::make_unsigned_t<T>;
        static constexpr auto conv(const T& v) const { return static_cast<type>(v); }
    };

    template<class T>
    struct make_signed_if_int<T, std::enable_if_t<is_native_integer_v<T>>>
    {
        using type = typename std::make_signed_t<T>;
    };

    // Compound integer types will be introduced on case by case basis

}; // namespace priv

// If integer, not bool, provide matching unsigned type, otherwise, leave type unchanged
// This name resembles its analog from STL, must always fully address it
template<class T>
using make_unsigned_if_integer_type = typename priv::make_unsigned_if_int<T>::type;

// If integer, not bool, provide matching signed type, otherwise, leave type unchanged
// Notably, it preserves signed status for floating-point types
// This name resembles its analog from STL, must always fully address it
template<class T>
using make_signed_if_integer_type = typename priv::make_signed_if_int<T>::type;

// Casts to itself unless T is signed integer type, in which case it returns unsigned value
// Note C++ rule 6.3.1.3 Signed and unsigned integers, paragraph 2:
// It adds (MAX+1) at conversion to any Negative value
template<class T>
constexpr auto to_unsigned_if_integer(const T& v)
{
    return priv::make_unsigned_if_int<T>::conv(v);
}

}; // namespace sklib

#endif // SKLIB_INCLUDED_TYPES_INTCONV_HPP

