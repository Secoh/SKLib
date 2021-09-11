// This file is part of SKLib: https://github.com/Secoh/SKLib
// Copyright [2020-2021] Secoh
//
// Licensed under the GNU Lesser General Public License, Version 2.1 or later. See: https://www.gnu.org/licenses/
// You may not use this file except in compliance with the License.
// Software is distributed on "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// Special exception from GNU LGPL terms: you don't have to publish the compiled object binary file(s) for SKLib.
// Modified source code and/or any derivative work requirements are still in effect. All such file(s) must be openly
// published under the same terms as the original one(s), but you don't have to inherit the special exception above.
//


#ifndef SKLIB_INCLUDED_UTILITY_HPP
#define SKLIB_INCLUDED_UTILITY_HPP

#ifndef SKLIB_PRELOADED_COMMON_HEADERS
#include <type_traits>
#include <string>
#endif


#define SKLIB_TYPES_IS_INTEGER(T)           (std::is_integral_v<T> && !std::is_same_v<T, bool>)
#define SKLIB_TYPES_IS_SIGNED_INTEGER(T)    (SKLIB_TYPES_IS_INTEGER(T) && std::is_signed_v<T>)
#define SKLIB_TYPES_IS_UNSIGNED_INTEGER(T)  (SKLIB_TYPES_IS_INTEGER(T) && std::is_unsigned_v<T>)
#define SKLIB_TYPES_IS_INTEGER_OF_SIZE(T,U) (SKLIB_TYPES_IS_INTEGER(T) && sizeof(T) == sizeof(U))
#define SKLIB_TYPES_IS_FLOATING_POINT(T)    (std::is_floating_point_v<T>)
#define SKLIB_TYPES_IS_NUMBER(T)            (SKLIB_TYPES_IS_INTEGER(T) || SKLIB_TYPES_IS_FLOATING_POINT(T))

#define SKLIB_TYPES_IS_CSTRING(T)           (std::is_pointer_v<T> && (std::is_same_v<std::remove_pointer_t<T>, char>))
#define SKLIB_TYPES_IS_WSTRING(T)           (std::is_pointer_v<T> && (std::is_same_v<std::remove_pointer_t<T>, wchar_t>))
#define SKLIB_TYPES_IS_STRING(T)            (SKLIB_TYPES_IS_CSTRING(T) || SKLIB_TYPES_IS_WSTRING(T))
#define SKLIB_TYPES_IS_STDCSTRING(T)        (std::is_same_v<T, std::string>)
#define SKLIB_TYPES_IS_STDWSTRING(T)        (std::is_same_v<T, std::wstring>)
#define SKLIB_TYPES_IS_STDSTRING(T)         (SKLIB_TYPES_IS_STDCSTRING(T) || SKLIB_TYPES_IS_STDWSTRING(T)) 
#define SKLIB_TYPES_IS_ANYSTRING(T)         (SKLIB_TYPES_IS_STRING(T) || SKLIB_TYPES_IS_STDSTRING(T))

#define SKLIB_INTERNAL_TEMPLATE_IF_INT_T    template<class T, std::enable_if_t<SKLIB_TYPES_IS_INTEGER(T), bool> = true>
#define SKLIB_INTERNAL_TEMPLATE_IF_UINT_T   template<class T, std::enable_if_t<SKLIB_TYPES_IS_UNSIGNED_INTEGER(T), bool> = true>

#define SKLIB_INTERNAL_FEATURE_IF_INT_T     SKLIB_INTERNAL_TEMPLATE_IF_INT_T static constexpr       /* useful shortcuts */
#define SKLIB_INTERNAL_FEATURE_IF_UINT_T    SKLIB_INTERNAL_TEMPLATE_IF_UINT_T static constexpr

#define SKLIB_INTERNAL_TEMPLATE_IF_INT_T_OF_SIZE(match_type)  template<class T, std::enable_if_t<SKLIB_TYPES_IS_INTEGER_OF_SIZE(T, match_type), bool> = true>





namespace sklib
{

// June 2021
// Current supporting compiler is C++14
// C++17 or C++20 features, however available on some platforms, are left for future SkLib versions
// when all relevant targets will catch up (notably Atmel Studio)
//

    namespace internal
    {

// Replacement for std::type_identity_t (C++20)
        template<class T> struct self_type { using type = T; };
        template<class T> using do_not_deduce = typename self_type<T>::type;

    };
};


#endif // SKLIB_INCLUDED_UTILITY_HPP
