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

#ifndef SKLIB_INCLUDED_UTILITY_HPP
#define SKLIB_INCLUDED_UTILITY_HPP

#ifndef SKLIB_PRELOADED_COMMON_HEADERS
#include <type_traits>
#include <string>
#include <iostream>
#include <iomanip>
#endif


#define SKLIB_TYPES_IS_INTEGER(T)           (std::is_integral_v<T> && !std::is_same_v<std::remove_cv_t<T>, bool>)
#define SKLIB_TYPES_IS_SIGNED_INTEGER(T)    (SKLIB_TYPES_IS_INTEGER(T) && std::is_signed_v<T>)
#define SKLIB_TYPES_IS_UNSIGNED_INTEGER(T)  (SKLIB_TYPES_IS_INTEGER(T) && std::is_unsigned_v<T>)
#define SKLIB_TYPES_IS_INTEGER_OF_SIZE(T,U) (SKLIB_TYPES_IS_INTEGER(T) && sizeof(T) == sizeof(U))
#define SKLIB_TYPES_IS_FLOATING_POINT(T)    (std::is_floating_point_v<T>)
#define SKLIB_TYPES_IS_NUMBER(T)            (SKLIB_TYPES_IS_INTEGER(T) || SKLIB_TYPES_IS_FLOATING_POINT(T))
#define SKLIB_TYPES_IS_ANYCHAR(T)           (std::is_same_v<std::remove_cv_t<T>, char> || std::is_same_v<std::remove_cv_t<T>, wchar_t>)

#define SKLIB_TYPES_IS_CSTRING(T)           (std::is_pointer_v<T> && (std::is_same_v<std::remove_cv_t<std::remove_pointer_t<T>>, char>))
#define SKLIB_TYPES_IS_WSTRING(T)           (std::is_pointer_v<T> && (std::is_same_v<std::remove_cv_t<std::remove_pointer_t<T>>, wchar_t>))
#define SKLIB_TYPES_IS_STRING(T)            (SKLIB_TYPES_IS_CSTRING(T) || SKLIB_TYPES_IS_WSTRING(T))
#define SKLIB_TYPES_IS_STDCSTRING(T)        (std::is_same_v<std::remove_cv_t<T>, std::string>)
#define SKLIB_TYPES_IS_STDWSTRING(T)        (std::is_same_v<std::remove_cv_t<T>, std::wstring>)
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
// Current supported language flavor is C++20

    namespace internal
    {
        template<class T> using do_not_deduce = typename std::type_identity_t<T>;

// Class quasi-member, global, and C-style programmable callbacks -- used in: bitwise
// NB: can be used as an "almost" drop-in replacement for a function address variable, with initialization, test for nullptr, and calling
        template<class CA, class FR, class ...FA>
        class callback_type
        {
        protected:
            typedef void (*cfunc_type)();                // pointer to non-member function (or static class member)

            typedef FR (*cfunc_self_type)(CA*, FA...);    // "normal" self-addressing callback
            typedef FR (*cfunc_bare_type)(FA...);         // simplified callback, good for single IO point
            typedef FR (*cfunc_void_type)(void*, FA...);  // general C-style callback with payload

            union { void* const descr = nullptr;  // pointer to external descriptor,
                    CA*   const root;  };         // or pointer to parent class, used mutually exclusively

            const cfunc_type func = nullptr;      // address of end-user callback - routed from member call() according to init options

            static FR routing_self(const callback_type* self, FA... args) { return ((cfunc_self_type)(self->func))(self->root, args...); }
            static FR routing_bare(const callback_type* self, FA... args) { return ((cfunc_bare_type)(self->func))(args...); }
            static FR routing_void(const callback_type* self, FA... args) { return ((cfunc_void_type)(self->func))(self->descr, args...); }
            static FR routing_dead(const callback_type* /*self*/, FA... /*args*/) { return FR(); }

            FR (* const reflector)(const callback_type*, FA...) = routing_dead;  // internal static function redirects call to requested parameters formatting

        public:
            explicit callback_type(cfunc_self_type f, CA* parent) : func((cfunc_type)f), reflector(routing_self), root(parent) {}
            explicit callback_type(cfunc_bare_type f)             : func((cfunc_type)f), reflector(routing_bare)               {}
            explicit callback_type(cfunc_void_type f, void* ext)  : func((cfunc_type)f), reflector(routing_void), descr(ext)   {}

            bool is_valid()     const { return func != nullptr; }                                  // verify whether constructor was called with invalid function address
            FR call(FA... args) const { return (is_valid() ? reflector(this, args...) : FR()); }   // make call to the "programmable" callback

            explicit operator bool()  const { return is_valid(); }
            FR operator()(FA... args) const { return call(args...); }
        };

        // for completeness, simplified callback classthat only does C-style callbacks
        template<class FR, class ...FA>
        class c_callback_type : public callback_type<int, FR, FA...>
        {
        protected:
            typedef FR (*cfunc_bare_type)(FA...);         // lets just copy that from the base class declaration
            typedef FR (*cfunc_void_type)(void*, FA...);  // to avoid declaring them "public" (namespace pollution)
        public:
            explicit c_callback_type(cfunc_bare_type f) : callback_type<int, FR, FA...>(f) {}
            explicit c_callback_type(cfunc_void_type f, void* ext) : callback_type<int, FR, FA...>(f, ext) {}
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

    // collection of strings at initialization

    namespace supplement
    {
        struct collection_cstring_check_type
        {};

        template<unsigned U>
        struct collection_cstring_type : public collection_cstring_check_type
        {
            static constexpr unsigned N = U;
            const char* text[N] = { nullptr };

            constexpr collection_cstring_type() = default;

            constexpr collection_cstring_type(const collection_cstring_type<std::max(N,2u)-1>& in)   // for completeness, only N >= 2 are used
            {
                for (unsigned k = 0; k < in.N; k++) text[k] = in.text[k];
            }

            constexpr void fill_array(const char* (&out)[N]) const
            {
                for (unsigned k = 0; k < N; k++) out[k] = text[k];
            }
        };
    };

    namespace internal
    {
        template<unsigned N>
        constexpr auto partial_collection_cstring(const ::sklib::supplement::collection_cstring_type<N>& A, const char* str)
        {
            ::sklib::supplement::collection_cstring_type<N+1> R{ A };
            R.text[N] = str;
            return R;
        }

        template<unsigned N, class ...Args>
        constexpr auto partial_collection_cstring(const ::sklib::supplement::collection_cstring_type<N>& A, const char* str, Args... args)
        {
            ::sklib::supplement::collection_cstring_type<N+1> R{ A };
            R.text[N] = str;
            return partial_collection_cstring(R, args...);
        }
    };

    template<class ...Args>
    constexpr auto collection_cstring(const char* str, Args... args)
    {
        ::sklib::supplement::collection_cstring_type<1> R;
        R.text[0] = str;
        return ::sklib::internal::partial_collection_cstring(R, args...);
    }

    template<>
    constexpr auto collection_cstring(const char* str)
    {
        ::sklib::supplement::collection_cstring_type<1> R;
        R.text[0] = str;
        return R;
    }

    constexpr auto collection_cstring()  // for completeness
    {
        return collection_cstring(nullptr);
    }

    // STDC string functions capable of constexpr

    template<class T, std::enable_if_t<SKLIB_TYPES_IS_ANYCHAR(T), bool> = true>
    constexpr size_t strlen(const T* str)
    {
        size_t N = 0;
        while (*str++) N++;
        return N;
    }

/* //sk?! do we need it?

    template<class T, class U, std::enable_if_t<(SKLIB_TYPES_IS_ANYCHAR(T) && SKLIB_TYPES_IS_INTEGER(U)), bool> = true>
    constexpr int strncmp(const T* stra, const T* strb, U n)
    {
        typedef std::make_unsigned_t<T> uT;
        static_assert(sizeof(T) == sizeof(uT), "SKLIB ** INTERNAL ERROR ** Signed and unsigned char (wchar_t) have different size. This cannot happen.");

        for (U i=0; i<n; i++)
        {
            uT ca = std::bit_cast<uT, T>(*stra++);   // compare to strcmp()
            uT cb = std::bit_cast<uT, T>(*strb++);

            if (!ca && !cb) return 0;  // equal

            if (!ca || ca < cb) return -1;
            if (!cb || ca > cb) return 1;
        }

        return 0;
    }

    template<class T, std::enable_if_t<SKLIB_TYPES_IS_ANYCHAR(T), bool> = true>
    constexpr int strcmp(const T* stra, const T* strb)
    {
        typedef std::make_unsigned_t<T> uT;
        static_assert(sizeof(T) == sizeof(uT), "SKLIB ** INTERNAL ERROR ** Signed and unsigned char (wchar_t) have different size. This cannot happen.");

        while (true)
        {
            uT ca = std::bit_cast<uT, T>(*stra++);   // compare to strncmp()
            uT cb = std::bit_cast<uT, T>(*strb++);

            if (!ca && !cb) return 0;  // equal

            if (!ca || ca < cb) return -1;
            if (!cb || ca > cb) return 1;
        }
    }
*/

    template<class T, class U, std::enable_if_t<(SKLIB_TYPES_IS_ANYCHAR(T) && SKLIB_TYPES_IS_INTEGER(U)), bool> = true>
    constexpr bool strnequ(const T* stra, const T* strb, U n)
    {
        for (U i=0; i<n; i++)
        {
            T ca = *stra++;   // compare to strequ()
            T cb = *strb++;

            if (!ca && !cb) return true;  // equal
            if (ca != cb) return false;   // that includes zero-nonzero condition
        }

        return true;
    }

    template<class T, std::enable_if_t<SKLIB_TYPES_IS_ANYCHAR(T), bool> = true>
    constexpr int strequ(const T* stra, const T* strb)
    {
        while (true)
        {
            T ca = *stra++;   // compare to strnequ()
            T cb = *strb++;

            if (!ca && !cb) return true;  // equal
            if (ca != cb) return false;   // that includes zero-nonzero condition
        }
    }

    // ASCII designators, (useful subset)

    namespace ascii
    {
        static constexpr int NUL = '\0';
        static constexpr int STX = '\x02';   // Start of Text
        static constexpr int ETX = '\x03';   // End of Text
        static constexpr int EOT = '\x04';   // End of Transmission
        static constexpr int ENQ = '\x05';   // Enquiry
        static constexpr int ACK = '\x06';   // Acknowledge
        static constexpr int NAK = '\x15';   // Negative Acknowledgement
        static constexpr int BEL = '\a';     // Bell
        static constexpr int BS  = '\b';
        static constexpr int TAB = '\t';
        static constexpr int LF  = '\n';
        static constexpr int CR  = '\r';
        static constexpr int VTAB= '\v';     // Vertical Tab
        static constexpr int FF  = '\f';     // Form Feed
        static constexpr int XON = '\x11';
        static constexpr int XOFF= '\x13';
        static constexpr int CAN = '\x18';   // Cancel
        static constexpr int ESC = '\x1B';
        static constexpr int DEL = '\x7F';
        static constexpr int SPC = ' ';      // Space
    }
};


#endif // SKLIB_INCLUDED_UTILITY_HPP
