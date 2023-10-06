// This file is part of SKLib: https://github.com/Secoh/SKLib
// Copyright [2020-2022] Secoh
//
// Licensed under the GNU Lesser General Public License, Version 2.1 or later. See: https://www.gnu.org/licenses/
// You may not use this file except in compliance with the License.
// Software is distributed on "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// Special exception from GNU LGPL terms: you don't have to publish the compiled object binary file(s) for SKLib.
// Modified source code and/or any derivative work requirements are still in effect. All such file(s) must be openly
// published under the same terms as the original one(s), but you don't have to inherit the special exception above.

// Provides flavor of strcmp() and strncmp() standard functions that can directly accept Unicode and its variety.
// This is internal SKLib file and must NOT be included directly.

// ==================================== Comparison

namespace opaque
{
    template<class V, std::enable_if_t<is_integer_val<V>, bool> = true>
    constexpr bool strcmp_impl_test_and_dec_if_int_else_true(V& n)
    {
        return (n-- > 0);
    }

    template<class V, std::enable_if_t<!is_integer_val<V>, bool> = true>
    constexpr bool strcmp_impl_test_and_dec_if_int_else_true(V&)
    {
        return true;
    }

    template<class T1, class T2, class V, bool Cap>
    constexpr int strcmp_impl(const T1* str1, const T2* str2, V n)
    {
        static_assert(is_integer_val<T1> && is_integer_val<T2>, "SKLIB ** Strings are represented by arrays of integers (e.g. chars)");
        static_assert(is_integer_val<V> || std::is_same_v<V, void*>, "SKLIB ** String length must be integer");

        typedef std::make_unsigned_t<T1> U1;
        typedef std::make_unsigned_t<T2> U2;

        while (strcmp_impl_test_and_dec_if_int_else_true<V>(n))  // accomodate both strcmp() and strncmp() versions
        {
            U1 c1 = static_cast<U1>(*str1++);
            U2 c2 = static_cast<U1>(*str2++);

            if (Cap)
            {
                c1 = ascii_toupper(c1);
                c2 = ascii_toupper(c2);
            }

            if (!c1 && !c2) return 0;  // equal

            if (c1 < c2) return -1;  // at this point, both chars cannot be zero
            if (c1 > c2) return 1;   // if one is zero, THEN another is nonzero => not equal => breaks the loop
        }

        return 0;
    }
};

// ------------------------------------  strcmp family

// Unicode-aware version of standard strcmp()
// Can directly accept ASCII, BMP, or full 4-byte Unicode in either argument
//
template<class T1, class T2>
constexpr auto strcmp(const T1* str1, const T2* str2)
{
    return ::sklib::opaque::strcmp_impl<T1, T2, void*, false>(str1, str2, nullptr);
}

// Unicode-aware version of standard stricmp() (or strcasecmp())
// Can directly accept ASCII, BMP, or full 4-byte Unicode in either argument
// Limitation: Can only convert ASCII letters between capital and lower case
// Cannot compare UTF-8 or UTF-16 data
//
template<class T1, class T2>
constexpr auto stracmp(const T1* str1, const T2* str2)
{
    return ::sklib::opaque::strcmp_impl<T1, T2, void*, true>(str1, str2, nullptr);
}

// Unicode-aware version of standard strncmp()
// Can directly accept ASCII, BMP, or full 4-byte Unicode in either argument
// Length can be any integer type
// Limitation: Cannot compare UTF-8 or UTF-16 data
//
template<class T1, class T2, class V>
constexpr auto strncmp(const T1* str1, const T2* str2, V n)
{
    return ::sklib::opaque::strcmp_impl<T1, T2, std::decay_t<V>, false>(str1, str2, n);
}

// Unicode-aware case-insensitive strncmp() (crossbreed with stricmp()/strcasecmp())
// Can directly accept ASCII, BMP, or full 4-byte Unicode in either argument
// Length can be any integer type
// Limitation: Can only convert ASCII letters between capital and lower case
// Cannot compare UTF-8 or UTF-16 data
//
template<class T1, class T2, class V>
constexpr auto strancmp(const T1* str1, const T2* str2, V n)
{
    return ::sklib::opaque::strcmp_impl<T1, T2, std::decay_t<V>, true>(str1, str2, n);
}

// ------------------------------------  strequ family, test for equilibrium (derivative)

// Test two strings for equilibrium
// Can directly accept ASCII, BMP, or full 4-byte Unicode in either argument
//
template<class T1, class T2>
constexpr bool strequ(const T1* str1, const T2* str2)
{
    return !strcmp(str1, str2);
}

// Case-insensitive test that two strings are equal
// Can directly accept ASCII, BMP, or full 4-byte Unicode in either argument
// Limitation: Can only convert ASCII letters between capital and lower case
// Cannot compare UTF-8 or UTF-16 data
//
template<class T1, class T2>
constexpr bool straequ(const T1* str1, const T2* str2)
{
    return !stracmp(str1, str2);
}

// Test that two strings are either equal, or have equal beginning of the given length
// Can directly accept ASCII, BMP, or full 4-byte Unicode in either argument
// Length can be any integer type
//
template<class T1, class T2, class V>
constexpr bool strnequ(const T1* str1, const T2* str2, V n)
{
    return !strncmp(str1, str2, n);
}

// Case-insensitive test that two strings are either equal, or have equal beginning of the given length
// Can directly accept ASCII, BMP, or full 4-byte Unicode in either argument
// Length can be any integer type
// Limitation: Can only convert ASCII letters between capital and lower case
// Cannot compare UTF-8 or UTF-16 data
//
template<class T1, class T2, class V>
constexpr bool stranequ(const T1* str1, const T2* str2, V n)
{
    return !strancmp(str1, str2, n);
}

// ==================================== Search

// ------------------------------------ future

// strchr, strachr, strnchr, stranchr, strrchr, strarchr
// strstr, strastr, strnstr, stranstr, strrstr, strarstr

// ==================================== Length

// Unicode-aware version of standard strlen()
// Limitation: Cannot handle UTF-8 or UTF-16 data
//
template<class V = size_t, class T = char>
constexpr auto strlen(const T* str)
{
    static_assert(is_integer_val<T>, "SKLIB ** Strings are represented by arrays of integers (e.g. chars)");

    std::decay_t<V> N = 0;
    while (*str++) N++;
    return N;
}

// ------------------------------------ strbeg family, test that string begins with smth

// Unicode-aware specialization of strncmp(): tests if str1 starts with str2
// Can directly accept ASCII, BMP, or full 4-byte Unicode in either argument
// Limitation: Cannot compare UTF-8 or UTF-16 data
// Primary purpose is to compare string with ASCII constant
//
template<class T1, class T2>
constexpr bool strbeg(const T1* str1, const T2* str2)
{
    const auto N = strlen(str2);
    return strnequ(str1, str2, N);
}

// Unicode-aware case-insensitive specialization of strncmp(): tests if str1 starts with str2
// Can directly accept ASCII, BMP, or full 4-byte Unicode in either argument
// Limitation: Can only convert ASCII letters between capital and lower case
// Cannot compare UTF-8 or UTF-16 data
// Primary purpose is to compare string with ASCII constant
//
template<class T1, class T2>
constexpr bool strabeg(const T1* str1, const T2* str2)
{
    const auto N = strlen(str2);
    return stranequ(str1, str2, N);
}


