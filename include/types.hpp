// This file is part of SKLib: https://github.com/Secoh/SKLib
// Copyright [2020-2022] Secoh
//
// Licensed under the GNU Lesser General Public License, Version 2.1 or later. See: https://www.gnu.org/licenses/
// You may not use this file except in compliance with the License.
// Software is distributed on "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// Special exception from GNU LGPL terms: you don't have to publish the compiled object binary file(s) for SKLib.
// Modified source code and/or any derivative work requirements are still in effect. All such file(s) must be openly
// published under the same terms as the original one(s), but you don't have to inherit the special exception above.
//

#ifndef SKLIB_INCLUDED_TYPES_HPP
#define SKLIB_INCLUDED_TYPES_HPP

#include <type_traits>

namespace sklib
{
    // June 2021
    // Current supported language flavor is C++20

    template<class T>
    inline constexpr bool is_integer_val = std::is_integral_v<T> && !std::is_same_v<std::remove_cv_t<T>, bool>;

    template<class T>
    inline constexpr bool is_signed_integer_val = is_integer_val<T> && std::is_signed_v<T>;

    template<class T>
    inline constexpr bool is_unsigned_integer_val = is_integer_val<T> && std::is_unsigned_v<T>;

    template<class T, class U>
    inline constexpr bool is_integer_of_size_val = is_integer_val<T> && sizeof(T) == sizeof(U);

    // is_floating_point_v - use STD

    template<class T>
    inline constexpr bool is_numeric_val = is_integer_val<T> || std::is_floating_point_v<T>;

    // strings (legacy; OK with ASCII or BMP Unicode)

    template<class T>
    inline constexpr bool is_any_char_val = std::is_same_v<std::remove_cv_t<T>, char> || std::is_same_v<std::remove_cv_t<T>, wchar_t>;

/* TODO
#define SKLIB_TYPES_IS_CSTRING(T)           (std::is_pointer_v<T> && (std::is_same_v<std::remove_cv_t<std::remove_pointer_t<T>>, char>))
#define SKLIB_TYPES_IS_WSTRING(T)           (std::is_pointer_v<T> && (std::is_same_v<std::remove_cv_t<std::remove_pointer_t<T>>, wchar_t>))
#define SKLIB_TYPES_IS_STRING(T)            (SKLIB_TYPES_IS_CSTRING(T) || SKLIB_TYPES_IS_WSTRING(T))
#define SKLIB_TYPES_IS_STDCSTRING(T)        (std::is_same_v<std::remove_cv_t<T>, std::string>)
#define SKLIB_TYPES_IS_STDWSTRING(T)        (std::is_same_v<std::remove_cv_t<T>, std::wstring>)
#define SKLIB_TYPES_IS_STDSTRING(T)         (SKLIB_TYPES_IS_STDCSTRING(T) || SKLIB_TYPES_IS_STDWSTRING(T)) 
#define SKLIB_TYPES_IS_ANYSTRING(T)         (SKLIB_TYPES_IS_STRING(T) || SKLIB_TYPES_IS_STDSTRING(T))
*/

// #define SKLIB_INTERNAL_TEMPLATE_IF_INT_T    template<class T, std::enable_if_t<SKLIB_TYPES_IS_INTEGER(T), bool> = true>
// #define SKLIB_INTERNAL_TEMPLATE_IF_UINT_T   template<class T, std::enable_if_t<SKLIB_TYPES_IS_UNSIGNED_INTEGER(T), bool> = true>

// #define SKLIB_INTERNAL_FEATURE_IF_INT_T     SKLIB_INTERNAL_TEMPLATE_IF_INT_T static constexpr       /* useful shortcuts */
// #define SKLIB_INTERNAL_FEATURE_IF_UINT_T    SKLIB_INTERNAL_TEMPLATE_IF_UINT_T static constexpr      /* "class feature" rather than just member item */

// #define SKLIB_INTERNAL_TEMPLATE_IF_INT_T_OF_SIZE(match_type)  template<class T, std::enable_if_t<SKLIB_TYPES_IS_INTEGER_OF_SIZE(T, match_type), bool> = true>


    namespace supplement
    {
        // if integer and not bool, return self; otherwise, return int (as just any random integer type)
        template<class T>
        using integer_or_int_type = typename std::conditional_t<is_integer_val<T>, T, int>;

        template<class T>
        class make_unsigned_if_integer
        {
        private:
            using T2 = typename std::make_unsigned_t<integer_or_int_type<T>>;
        public:
            using type = typename std::conditional_t<is_integer_val<T>, T2, T>;
        };

        template<class T>
        class make_signed_if_integer
        {
        private:
            using T2 = typename std::make_signed_t<integer_or_int_type<T>>;
        public:
            using type = typename std::conditional_t<is_integer_val<T>, T2, T>;
        };
    };

    // if integer, not bool, provide matching unsigned type, otherwise, leave type unchanged
    template<class T>
    using make_unsigned_if_integer_type = typename supplement::make_unsigned_if_integer<T>::type;

    // if integer, not bool, provide matching signed type, otherwise, leave type unchanged
    // notably, it preserves signed status for floating-point types
    template<class T>
    using make_signed_if_integer_type = typename supplement::make_signed_if_integer<T>::type;

    // Casts to itself unless T is signed integer type, in which case it returns unsigned value
    // Note C++ rule 6.3.1.3 Signed and unsigned integers, paragraph 2:
    // It adds (MAX+1) at conversion to any Negative value
    template<class T>
    constexpr auto to_unsigned_if_integer(const T& v)
    {
        return static_cast<make_unsigned_if_integer_type<T>>(v);
    }
};

#endif // SKLIB_INCLUDED_TYPES_HPP

