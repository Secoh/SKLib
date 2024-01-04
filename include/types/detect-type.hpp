// This file is part of SKLib: https://github.com/Secoh/SKLib
// Copyright [2020-2023] Secoh
//
// Licensed under the GNU Lesser General Public License, Version 2.1 or later. See: https://www.gnu.org/licenses/
// You may not use this file except in compliance with the License.
// Software is distributed on "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// Special exception from GNU LGPL terms: you don't have to publish the compiled object binary file(s) for SKLib.
// Modified source code and/or any derivative work requirements are still in effect. All such file(s) must be openly
// published under the same terms as the original one(s), but you don't have to inherit the special exception above.

// Provides constants, macros, and templates for detection of C/C++ types.
// This is internal SKLib file and must NOT be included directly.

template<class T>
inline constexpr bool is_integer_v = std::is_integral_v<T> && !std::is_same_v<std::remove_cv_t<T>, bool>;

template<class T>
inline constexpr bool is_integer_pointer_or_nullptr_v =
    (std::is_pointer_v<T> && is_integer_v<std::remove_pointer_t<T>>) || std::is_null_pointer_v<T>;

template<class T>
inline constexpr bool is_signed_integer_v = sklib::is_integer_v<T> && std::is_signed_v<T>;

template<class T>
inline constexpr bool is_unsigned_integer_v = sklib::is_integer_v<T> && std::is_unsigned_v<T>;

template<class T, class U>
inline constexpr bool is_integer_of_size_v = sklib::is_integer_v<T> && sizeof(T) == sizeof(U);

template<class T>
inline constexpr bool is_floating_point_v = std::is_floating_point_v<T>; // for completeness

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
inline constexpr bool is_stringz = sklib::is_cstringz<T> || sklib::is_wstringz<T>;

#ifndef SKLIB_TARGET_MCU

template<class T>
inline constexpr bool is_std_cstring = std::is_same_v<std::remove_cv_t<T>, std::string>;

template<class T>
inline constexpr bool is_std_wstring = std::is_same_v<std::remove_cv_t<T>, std::wstring>;

template<class T>
inline constexpr bool is_std_string = sklib::is_std_cstring<T> || sklib::is_std_wstring<T>;

template<class T>
inline constexpr bool is_any_string = sklib::is_stringz<T> || sklib::is_std_string<T>;

#else

template<class T>
inline constexpr bool is_any_string = sklib::is_stringz<T>;

#endif

// conditional template declarations

#define SKLIB_INTERNAL_ENABLE_IF_CONDITION(Test)    std::enable_if_t<(Test), bool> = true

//sk update!
#define SKLIB_INTERNAL_ENABLE_IF_INT(T)    std::enable_if_t<sklib::is_integer_v<T>, bool> = true
#define SKLIB_INTERNAL_ENABLE_IF_UINT(T)   std::enable_if_t<sklib::is_unsigned_integer_v<T>, bool> = true
#define SKLIB_INTERNAL_ENABLE_IF_SINT(T)   std::enable_if_t<sklib::is_signed_integer_v<T>, bool> = true

#define SKLIB_INTERNAL_ENABLE_IF_DERIVED(T,BAS) std::enable_if_t<std::is_base_of_v<BAS, T>, bool> = true

#define SKLIB_INTERNAL_ENABLE_IF_INT_OF_SIZE(T,match_type) \
std::enable_if_t<sklib::is_integer_of_size_v<T, match_type>, bool> = true

#define SKLIB_TEMPLATE_IF_INT(T)    template<class T, SKLIB_INTERNAL_ENABLE_IF_INT(T)>
#define SKLIB_TEMPLATE_IF_UINT(T)   template<class T, SKLIB_INTERNAL_ENABLE_IF_UINT(T)>
#define SKLIB_TEMPLATE_IF_SINT(T)   template<class T, SKLIB_INTERNAL_ENABLE_IF_SINT(T)>

#define SKLIB_TEMPLATE_IF_NOT_INT(T) template<class T, std::enable_if_t<!sklib::is_integer_v<T>, bool> = true>

#define SKLIB_TEMPLATE_IF_DERIVED(T,BAS) template<class T, SKLIB_INTERNAL_ENABLE_IF_DERIVED(T,BAS)>

#define SKLIB_TEMPLATE_IF_INT_OF_SIZE(T,match_type) \
template<class T, SKLIB_INTERNAL_ENABLE_IF_INT_OF_SIZE(T,match_type)>

