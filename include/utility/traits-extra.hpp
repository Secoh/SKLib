// This file is part of SKLib: https://github.com/Secoh/SKLib
// Copyright [2020-2022] Secoh
//
// Licensed under the GNU Lesser General Public License, Version 2.1 or later. See: https://www.gnu.org/licenses/
// You may not use this file except in compliance with the License.
// Software is distributed on "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// Special exception from GNU LGPL terms: you don't have to publish the compiled object binary file(s) for SKLib.
// Modified source code and/or any derivative work requirements are still in effect. All such file(s) must be openly
// published under the same terms as the original one(s), but you don't have to inherit the special exception above.

// -------------------------------------------------------
// Templates that should have been in standard <type_traits>

// -------------------------------------------------------
// 1. Distinguish on consteval level whether a type is forward-declared of fully defined

template<class T, class = void>
struct is_complete : std::false_type {};

template<class T>
struct is_complete<T, decltype(void(sizeof(T)))> : std::true_type {};

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
// 2. Provide member type for a class depending on class declaration, so the name of the type is known

#define SKLIB_SUPPLEMENT_DECLARE_TYPE_CONDUIT(type_conduit_name,type_reflection_name) \
template<class T> class type_conduit_name { protected: typedef T type_reflection_name; };

SKLIB_SUPPLEMENT_DECLARE_TYPE_CONDUIT(type_conduit, reflection_type)

// Example how to use
// -----------------------
// Class X receives "self" member type regardless of T:
// template<class T> class X : public type_conduit<X<T>>
//
// Class Myclass receives member type with predefine name, same as provided to its base class:
// #define MYCLASS(tname) class Myclass : public X<tname>, public type_conduit<tname>
// 
// Reference: https://stackoverflow.com/questions/21143835/can-i-implement-an-autonomous-self-member-type-in-c


