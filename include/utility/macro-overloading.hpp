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
// Provides means and example how to declare C macros, "overloaded" by number of parameters.
// This is internal SKLib file and must NOT be included directly.

#define SKLIB_SUPPLEMENT_EXPAND_COMMA(...) __VA_ARGS__

#ifdef _MSVC_TRADITIONAL
#if _MSVC_TRADITIONAL

#undef SKLIB_SUPPLEMENT_EXPAND_COMMA
#define SKLIB_SUPPLEMENT_EXPAND_COMMA(that) that

#endif
#endif

#define SKLIB_MACRO_SELF(that) that
#define SKLIB_MACRO_CONCAT_TWO(one,two) one##two

#define SKLIB_INTERNAL_MACRO_SUBSTITUTE_ONE_TWO(one,two,what,...) what
#define SKLIB_MACRO_SELECT_ONE_TWO(one,two,...) SKLIB_SUPPLEMENT_EXPAND_COMMA(SKLIB_INTERNAL_MACRO_SUBSTITUTE_ONE_TWO(__VA_ARGS__, two, one, dummy)(__VA_ARGS__))

// Example how to use
// -----------------------
//
// This is what we need to accomplish, "overloaded" macros:
//
//   #define DECLARE(name)      struct name : public base_class<char>
//   #define DECLARE(name,type) struct name : public base_class<type>
//
// Implementation
//
//   #define INTERNAL_DECLARE_ONE(name)      struct name : public base_class<char>
//   #define INTERNAL_DECLARE_TWO(name,type) struct name : public base_class<type>
//   #define DECLARE(...) SKLIB_MACRO_SELECT_ONE_TWO(INTERNAL_DECLARE_ONE, INTERNAL_DECLARE_TWO, __VA_ARGS__)
//
// Reference: https://stackoverflow.com/questions/11761703/overloading-macro-on-number-of-arguments

