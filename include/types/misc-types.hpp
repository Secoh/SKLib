// This file is part of SKLib: https://github.com/Secoh/SKLib
// Copyright [2020-2025] Secoh
//
// Licensed under the GNU Lesser General Public License, Version 2.1 or later. See: https://www.gnu.org/licenses/
// You may not use this file except in compliance with the License.
// Software is distributed on "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// Special exception from GNU LGPL terms: you don't have to publish the compiled object binary file(s) for SKLib.
// Modified source code and/or any derivative work requirements are still in effect. All such file(s) must be openly
// published under the same terms as the original one(s), but you don't have to inherit the special exception above.

#ifndef SKLIB_INCLUDED_TYPES_MISC_HPP
#define SKLIB_INCLUDED_TYPES_MISC_HPP

#include <type_traits>

#include "int-bits.hpp"

namespace sklib {

// Uncategorized stuff related to C/C++ types.
// This is internal SKLib file and must NOT be included directly.

namespace aux
{
// -------------------------------------------------------
// Self-explanatory name for standard STL template

    template<class T> using do_not_deduce = typename std::type_identity_t<T>;

// -------------------------------------------------------
// Distinguish on consteval level whether a type is forward-declared of fully defined

    template<class T, class = void> struct is_complete : std::false_type {};
    template<class T> struct is_complete<T, decltype(void(sizeof(T)))> : std::true_type {};

// Example how to use
// -----------------------
// The template allows us to distinguish between forward declaration of a type, and the full declaration.
// It may be used, for example, as the constexpr function that shows, in compile time, the order of C/C++
// tokens seen in parse time.
//
//   struct example;                                          // forward declaration
//   static constexpr auto f1 = is_complete<example>::value;  // false
//   struct example { int a };                                // type is created
//   static constexpr auto f2 = is_complete<example>::value;  // true
//
// NB: Beware, the specific implementation in a comiler may be buggy.
//
// Reference: https://stackoverflow.com/questions/57624408/sfinae-detect-if-type-is-defined
//            https://devblogs.microsoft.com/oldnewthing/20190710-00/?p=102678 (Raymond Chen)

// -------------------------------------------------------
// Equivalent of std::array, possibly verbatim
// However, we do need guarantee that it uses only C/C++ language structure and keywords
// Thus, the following explicit re-implementation

    template<class T, size_t N> struct encapsulated_array_type { T data[N]; };
    template<class T> using encapsulated_array_octet_index_type = encapsulated_array_type<T, OCTET_ADDRESS_SPAN>;
    template<class T> using encapsulated_array_width_index_type = encapsulated_array_type<T, bits_width_v<T>>;

}; // namespace aux

}; // namespace sklib

#endif // SKLIB_INCLUDED_TYPES_MISC_HPP

