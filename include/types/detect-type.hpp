// This file is part of SKLib: https://github.com/Secoh/SKLib
// Copyright [2020-2025] Secoh
//
// Licensed under the GNU Lesser General Public License, Version 2.1 or later. See: https://www.gnu.org/licenses/
// You may not use this file except in compliance with the License.
// Software is distributed on "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// Special exception from GNU LGPL terms: you don't have to publish the compiled object binary file(s) for SKLib.
// Modified source code and/or any derivative work requirements are still in effect. All such file(s) must be openly
// published under the same terms as the original one(s), but you don't have to inherit the special exception above.

#ifndef SKLIB_INCLUDED_TYPES_DETECT_HPP
#define SKLIB_INCLUDED_TYPES_DETECT_HPP

#include <type_traits>

#include "../configure.hpp"

#ifndef SKLIB_TARGET_MCU
#include <string>
#endif

namespace sklib {

// Provides constants, macros, and templates for detection of C/C++ types.
// Upd Dec 2024: Added support for compound (non-standard, artificial) integers
// This is internal SKLib file and must NOT be included directly.

// Conditional templates helper
#define SKLIB_TYPE_ENABLE_IF_CONDITION(rtype,condition) std::enable_if_t<condition, rtype>

// -------------------------------------------------------------------------
// Integers

namespace aux
{
    struct compound_integer_anchor {};

    template<unsigned unsigned_bits_count, bool is_type_signed>
    struct compound_integer_base : public priv::compound_integer_anchor
    {
        static constexpr unsigned positive_width = unsigned_bits_count;
        static constexpr bool has_sign = is_type_signed;
    };
};

template<class T>
inline constexpr bool is_native_integer_v = std::is_integral_v<T> && !std::is_same_v<std::remove_cv_t<T>, bool>;

template<class T>
inline constexpr bool is_compound_integer_v = std::is_base_of_v<aux::compound_integer_anchor, T>;

// This name resembles its analog from STL, must always fully address it
template<class T>
inline constexpr bool is_integer_v = is_native_integer_v<T> || is_compound_integer_v<T>;

#define SKLIB_TYPE_ENABLE_IF_INT(rtype,T)          SKLIB_TYPE_ENABLE_IF_CONDITION(rtype, sklib::is_integer_v<T>)
#define SKLIB_TYPE_ENABLE_IF_NATIVE_INT(rtype,T)   SKLIB_TYPE_ENABLE_IF_CONDITION(rtype, sklib::is_native_integer_v<T>)
#define SKLIB_TYPE_ENABLE_IF_COMPOUND_INT(rtype,T) SKLIB_TYPE_ENABLE_IF_CONDITION(rtype, sklib::is_compound_integer_v<T>)

namespace priv
{
    template<class T> constexpr SKLIB_TYPE_ENABLE_IF_NATIVE_INT(bool, T) is_signed()   { return std::is_signed_v<T>; }
    template<class T> constexpr SKLIB_TYPE_ENABLE_IF_COMPOUND_INT(bool, T) is_signed() { return T::has_sign; }
};

// This name resembles its analog from STL, must always fully address it
template<class T> inline constexpr bool is_signed_v = priv::is_signed<T>();

//sk do we need this at all??!
//template<class T>
//inline constexpr bool is_integer_pointer_or_nullptr_v =
//    (std::is_pointer_v<T> && sklib::is_integer_v<std::remove_pointer_t<T>>) || std::is_null_pointer_v<T>;

template<class T>
inline constexpr bool is_signed_integer_v = sklib::is_integer_v<T> && sklib::is_signed_v<T>;

template<class T>
inline constexpr bool is_unsigned_integer_v = sklib::is_integer_v<T> && !sklib::is_signed_v<T>;

template<class T>
inline constexpr bool is_signed_native_integer_v = is_native_integer_v<T> && sklib::is_signed_v<T>;

template<class T>
inline constexpr bool is_unsigned_native_integer_v = is_native_integer_v<T> && !sklib::is_signed_v<T>;

#define SKLIB_TYPE_ENABLE_IF_UINT(rtype,T) SKLIB_TYPE_ENABLE_IF_CONDITION(rtype, sklib::is_unsigned_integer_v<T>)
#define SKLIB_TYPE_ENABLE_IF_SINT(rtype,T) SKLIB_TYPE_ENABLE_IF_CONDITION(rtype, sklib::is_signed_integer_v<T>)

#define SKLIB_TYPE_ENABLE_IF_NATIVE_UINT(rtype,T) SKLIB_TYPE_ENABLE_IF_CONDITION(rtype, sklib::is_unsigned_native_integer_v<T>)
#define SKLIB_TYPE_ENABLE_IF_NATIVE_SINT(rtype,T) SKLIB_TYPE_ENABLE_IF_CONDITION(rtype, sklib::is_signed_native_integer_v<T>)

//sk looks as ill-formed idea
//template<class T, class U> inline constexpr bool is_integer_of_size_v = sklib::is_integer_v<T> && sizeof(T) == sizeof(U);

// -------------------------------------------------------------------------
// Other arithmetic    //sk???

// This name resembles its analog from STL, must always fully address it
template<class T>
inline constexpr bool is_floating_point_v = std::is_floating_point_v<T>; // for completeness

// This name resembles its analog from STL, must always fully address it
template<class T>
inline constexpr bool is_numeric_v = sklib::is_integer_v<T> || std::is_floating_point_v<T>;

// strings (legacy; OK with ASCII or BMP Unicode)

template<class T>
inline constexpr bool is_any_char_v = std::is_same_v<std::remove_cv_t<T>, char> ||
std::is_same_v<std::remove_cv_t<T>, wchar_t>;

template<class T>
inline constexpr bool is_cstringz = std::is_pointer_v<T> &&
std::is_same_v<std::remove_cv_t<std::remove_pointer_t<T>>, char>;

template<class T>
inline constexpr bool is_wstringz = std::is_pointer_v<T> &&
std::is_same_v<std::remove_cv_t<std::remove_pointer_t<T>>, wchar_t>;

template<class T>
inline constexpr bool is_stringz = is_cstringz<T> || is_wstringz<T>;

#ifndef SKLIB_TARGET_MCU

template<class T>
inline constexpr bool is_std_cstring = std::is_same_v<std::remove_cv_t<T>, std::string>;

template<class T>
inline constexpr bool is_std_wstring = std::is_same_v<std::remove_cv_t<T>, std::wstring>;

template<class T>
inline constexpr bool is_std_string = is_std_cstring<T> || is_std_wstring<T>;

template<class T>
inline constexpr bool is_any_string = is_stringz<T> || is_std_string<T>;

#else // if not SKLIB_TARGET_MCU

template<class T>
inline constexpr bool is_any_string = is_stringz<T>;

#endif // SKLIB_TARGET_MCU

//sk!!! rework: the entire concept is wrong: cannot declare templated class with conditional template
// (however, SFINAE in the return type in function declaration is legal

// conditional template declarations

/*

xxx #define SKLIB_INTERNAL_ENABLE_IF_CONDITION(Test)    std::enable_if_t<(Test), bool> = true

//sk update!
xxx #define SKLIB_INTERNAL_ENABLE_IF_INT(T)    std::enable_if_t<sklib::is_integer_v<T>, bool> = true
xxx #define SKLIB_INTERNAL_ENABLE_IF_UINT(T)   std::enable_if_t<sklib::is_unsigned_integer_v<T>, bool> = true
// none #define SKLIB_INTERNAL_ENABLE_IF_SINT(T)   std::enable_if_t<sklib::is_signed_integer_v<T>, bool> = true

#define SKLIB_INTERNAL_ENABLE_IF_DERIVED(T,BAS) std::enable_if_t<std::is_base_of_v<BAS, T>, bool> = true

#define SKLIB_INTERNAL_ENABLE_IF_INT_OF_SIZE(T,match_type) \
std::enable_if_t<sklib::is_integer_of_size_v<T, match_type>, bool> = true

xxx #define SKLIB_TEMPLATE_IF_INT(T)    template<class T, SKLIB_INTERNAL_ENABLE_IF_INT(T)>
xxx #define SKLIB_TEMPLATE_IF_UINT(T)   template<class T, SKLIB_INTERNAL_ENABLE_IF_UINT(T)>
xxx >edit! #define SKLIB_TEMPLATE_IF_SINT(T)   template<class T, SKLIB_INTERNAL_ENABLE_IF_SINT(T)>

#define SKLIB_TEMPLATE_IF_NOT_INT(T) template<class T, std::enable_if_t<!sklib::is_integer_v<T>, bool> = true>

#define SKLIB_TEMPLATE_IF_DERIVED(T,BAS) template<class T, SKLIB_INTERNAL_ENABLE_IF_DERIVED(T,BAS)>

xxx #define SKLIB_TEMPLATE_IF_INT_OF_SIZE(T,match_type) \
xxx template<class T, SKLIB_INTERNAL_ENABLE_IF_INT_OF_SIZE(T,match_type)>

*/

}; // namespace sklib

#endif // SKLIB_INCLUDED_TYPES_DETECT_HPP

