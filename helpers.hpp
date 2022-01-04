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
#include <iostream>
#include <iomanip>
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
#define SKLIB_INTERNAL_FEATURE_IF_UINT_T    SKLIB_INTERNAL_TEMPLATE_IF_UINT_T static constexpr      /* "class feature" rather than just member item */

#define SKLIB_INTERNAL_TEMPLATE_IF_INT_T_OF_SIZE(match_type)  template<class T, std::enable_if_t<SKLIB_TYPES_IS_INTEGER_OF_SIZE(T, match_type), bool> = true>



namespace sklib
{

// June 2021
// Current supported language flavor is C++14
// C++17 or C++20 features, however available on some platforms, are left for future SkLib versions
// when all relevant targets will catch up (notably Atmel Studio)
//

    namespace internal
    {
// Replacement for std::type_identity_t (C++20)
        template<class T> struct self_type { using type = T; };
        template<class T> using do_not_deduce = typename self_type<T>::type;

// Class quasi-member, global, and C-style programmable callbacks -- used in: bitwise
        template<class CA, class FR, class ...FA>
        class callback_type
        {
        public:
            typedef void (*cfunc_type)();                // pointer to non-member function (or static class member)

            typedef FR(*cfunc_self_type)(CA*, FA...);    // "normal" self-addressing callback
            typedef FR(*cfunc_bare_type)(FA...);         // simplified callback, good for single IO point
            typedef FR(*cfunc_void_type)(void*, FA...);  // general C-style callback with payload

        private:
            union { void* const descr = nullptr;  // pointer to external descriptor,
                    CA*   const root;  };         // or pointer to parent class, used mutually exclusively

            const cfunc_type func;                // address of end-user callback - routed from member call() according to init options

            static FR routing_self(const callback_type* self, FA... args) { return ((cfunc_self_type)(self->func))(self->root, args...); }
            static FR routing_bare(const callback_type* self, FA... args) { return ((cfunc_bare_type)(self->func))(args...); }
            static FR routing_void(const callback_type* self, FA... args) { return ((cfunc_void_type)(self->func))(self->descr, args...); }

            FR (* const reflector)(const callback_type*, FA...);  // internal static function redirects call to requested parameters formatting

        public:
            explicit callback_type(cfunc_self_type f, CA* parent) : func((cfunc_type)f), reflector(routing_self), root(parent) {}
            explicit callback_type(cfunc_bare_type f)             : func((cfunc_type)f), reflector(routing_bare)               {}
            explicit callback_type(cfunc_void_type f, void* ext)  : func((cfunc_type)f), reflector(routing_void), descr(ext)   {}

            bool is_valid()     const { return func != nullptr; }           // verify whether constructor was called with invalid function address
            FR call(FA... args) const { return reflector(this, args...); }  // make call to the "programmable" callback
        };


//sk: on how to make deducible function arguments from function address type, in template - for future reference
//        template<class CA, class FN> struct callback_type;
//        template<class CA, class FR, class ...FA>
//        struct callback_type<CA, FR(*)(FA...)>        { ... };

    };


    namespace supplement
    {
        template<class T>
        static constexpr void table256_print(const std::string& title, const T* U, int width, bool hex = false, int hex_digits = 2)
        {
            std::cout << title << "\n";

            for (int k=0; k<256; )
            {
                for (int i=0; i<width; i++)
                {
                    if (i) std::cout << " ";

                    if (!hex)
                    {
                        std::cout << (U[k++] * 1);    // <-- use integer promotion to eliminate "char" conversion on output
                    }
                    else
                    {
                        std::cout << "0x" << std::setfill('0') << std::setw(hex_digits) << std::hex << std::uppercase << (U[k++] * 1);
                    }

                    if (k<256) std::cout << ",";
                }
                std::cout << "\n";
            }
        }
    };
};


#endif // SKLIB_INCLUDED_UTILITY_HPP
