// This file is part of SKLib: https://github.com/Secoh/SKLib
// Copyright [2020-2025] Secoh
//
// Licensed under the GNU Lesser General Public License, Version 2.1 or later. See: https://www.gnu.org/licenses/
// You may not use this file except in compliance with the License.
// Software is distributed on "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// Special exception from GNU LGPL terms: you don't have to publish the compiled object binary file(s) for SKLib.
// Modified source code and/or any derivative work requirements are still in effect. All such file(s) must be openly
// published under the same terms as the original one(s), but you don't have to inherit the special exception above.

#ifndef SKLIB_INCLUDED_TYPES_INTBITS_HPP
#define SKLIB_INCLUDED_TYPES_INTBITS_HPP

#include <limits>
#include <type_traits>

#include "detect-type.hpp"

namespace sklib {

// Provides constants and functions related to bits in integers as fundamental C types, placement, count, etc.
// This is internal SKLib file and must NOT be included directly.

inline constexpr unsigned OCTET_BITS = 8;

static_assert(sizeof(char) == 1, "SKLIB ** INTERNAL ERROR ** sizeof(char) must be equal to 1");
static_assert(CHAR_BIT >= OCTET_BITS, "SKLIB ** INTERNAL ERROR ** char size must be at least 8 bits");

namespace priv
{
    template<class T, typename = void>
    struct int_bit_state
    {
        static_assert(false, "SKLIB ** INTERNAL ERROR ** Integer bit placement is defined only for integer types");
    };

    template<class T>
    struct int_bit_state<T, std::enable_if_t<is_native_integer_v<T>>>
    {
        static constexpr T data_cap(unsigned width) { return (T(1) << width); }
    };

    // Each compound integer implementation will provide its own bit-state function
    // template<class T> struct integer_bit_state<T, std::enable_if_t<is_compound_integer_v<T>>> {};
};

template<class T> constexpr SKLIB_TYPE_ENABLE_IF_INT(T, T) bits_data_cap(unsigned width)
{
    return priv::int_bit_state<T>::data_cap(width);
}

template<class T> constexpr SKLIB_TYPE_ENABLE_IF_INT(T, T) bits_data_high_1(unsigned width)
{
    return (width ? bits_data_cap<T>(width-1) : T(0));
}

namespace priv
{
    template<class T> consteval T data_mask(unsigned width)
    {
        auto A = bits_data_high_1<T>(width);
        return A | (A-1);
    }

    template<class T, typename = void>
    struct int_features
    {
        static_assert(false, "SKLIB ** INTERNAL ERROR ** Integer bit span is defined only for integer types");
    };

    template<class T>
    struct int_features<T, std::enable_if_t<is_native_integer_v<T>>>
    {
        // guaranteed by these definitions: unsigned int bit count is always even,
        // and positive_width always equals to total_width for unsigned types
        static constexpr unsigned total_width = OCTET_BITS * (unsigned)sizeof(T);
        static constexpr unsigned positive_width = total_width - (std::is_signed_v<T> ? 1 : 0);

        static constexpr T mask = data_mask<T>(positive_width);
        static constexpr T maxp = std::numeric_limits<T>::max();
        static_assert(mask > 0 && ((CHAR_BIT == OCTET_BITS) ? (mask == maxp) : (mask <= maxp)),
            "SKLIB ** INTERNAL ERROR ** Calculated integer bounds are outside std::limits while evaluating native_int_features");
    };

    template<class T>
    struct int_features<T, std::enable_if_t<is_compound_integer_v<T>>>
    {
        static constexpr unsigned total_width = T::positive_width;
        static constexpr unsigned positive_width = total_width;
    };

/* // sk do we need this?
    template<class T>
    constexpr SKLIB_TYPE_ENABLE_IF_CONDITION(unsigned, sklib::is_native_integer_v<T>) bits_total_width()
    { return native_int_features<T>::total_width; }

    template<class T>
    constexpr SKLIB_TYPE_ENABLE_IF_CONDITION(unsigned, sklib::is_native_integer_v<T>) bits_width()
    { return native_int_features<T>::positive_width; }

    template<class T>
    constexpr SKLIB_TYPE_ENABLE_IF_CONDITION(unsigned, sklib::is_compound_integer_v<T>) bits_width()
    { return T::positive_width; }
*/
};

// Usable bits count of the integer for safely mixing arithmetic and bitwise operations
// Fundamental unsigned integers - the bit count of the data type
// Fundamental signed integers - the number of bits that can represent positive number
// SKLib compound integer - the declared bit width
// All other types - compilation error
template<class T>
inline constexpr unsigned bits_width_v = priv::int_features<T>::positive_width;

template<class T> constexpr SKLIB_TYPE_ENABLE_IF_INT(T, T) bits_data_mask(unsigned width)
{
    if (!width) return T(0);
    if (width >= bits_width_v<T>) return priv::data_mask<T>(bits_width_v<T>);
    return bits_data_cap<T>(width) - 1;
}

template<class T>
inline constexpr T bits_high_1_v = bits_data_high_1<T>(bits_width_v<T>);

template<class T>
inline constexpr T bits_mask_v = bits_data_mask<T>(bits_width_v<T>);

inline constexpr uint8_t OCTET_MASK = bits_mask_v<uint8_t>;
inline constexpr size_t  OCTET_ADDRESS_SPAN = OCTET_MASK + 1;

namespace priv
{
    template<class T, unsigned Width> consteval SKLIB_TYPE_ENABLE_IF_INT(T, T) data_cap_select()
    {
        static_assert(Width < bits_width_v<T>,
            "SKLIB ** INTERNAL ERROR ** Size of data type must be enough to hold specified number of bits");
        return bits_data_cap<T>(Width);
    }

    template<class T, unsigned Width> consteval SKLIB_TYPE_ENABLE_IF_INT(T, T) data_high_1_select()
    {
        static_assert(Width > 0, "SKLIB ** INTERNAL ERROR ** Bit count must be nonzero");
        static_assert(Width <= bits_width_v<T>,
            "SKLIB ** INTERNAL ERROR ** Size of data type must be enough to hold specified number of bits");
        return bits_data_high_1<T>(Width);
    }

    template<class T, unsigned Width> consteval SKLIB_TYPE_ENABLE_IF_INT(T, T) data_mask_select()
    {
        auto A = data_high_1_select<T, Width>();
        return A | (A-1);
    }

    template<class T, typename = void>
    struct int_even_length
    {
        static_assert(false, "SKLIB ** INTERNAL ERROR ** Expected integer with even bit count");
    };
    template<class T>
    struct int_even_length<T, std::enable_if_t<bits_width_v<T> % 2 == 0>>
    {
        consteval unsigned half_length() { return bits_width_v<T> / 2; }
    };

//sk delete    template<class T>
//    constexpr SKLIB_TYPE_ENABLE_IF_CONDITION(unsigned, (bits_width_v<T> % 2 == 0)) half_length()
//    {
//        return bits_width_v<T> / 2;
//    }
};

template<class T, unsigned Width>
inline constexpr T bits_data_cap_v = priv::data_cap_select<T, Width>();
template<class T, unsigned Width>
inline constexpr T bits_data_high_1_v = priv::data_high_1_select<T, Width>();
template<class T, unsigned Width>
inline constexpr T bits_data_mask_v = priv::data_mask_select<T, Width>();

template<class T>
inline constexpr T bits_low_half_v = bits_data_mask<T>(priv::int_even_length<T>::half_length());
template<class T>
inline constexpr T bits_high_half_v = (bits_low_half_v<T> << priv::int_even_length<T>::half_length());


// how it works: if high and low bounds are the same, enable if T is equal or smaller than bound
// if not the same, enable if T is strictly greater than low bound, and equal or smaller than high bound
#define SKLIB_TYPE_ENABLE_IF_NATIVE_INT_OF_SIZE(rtype,T,low_bound_type,high_bound_type)                                     \
    SKLIB_TYPE_ENABLE_IF_CONDITION(rtype, (sklib::bits_width_v<T> <= sklib::bits_width_v<high_bound_type> &&                \
                                           (sklib::bits_width_v<low_bound_type> == sklib::bits_width_v<high_bound_type>     \
                                                ? true : sklib::bits_width_v<low_bound_type> < sklib::bits_width_v<T>)))



//sk   SKLIB_TEMPLATE_IF_INT(T) inline constexpr unsigned bits_width_less_sign_v = bits_width_v<T> - (sklib::is_signed_v<T> ? 1 : 0);


#ifdef SKEN

/* //sk don't need
namespace priv
{

}; // namespace priv
*/

// bits by count (for container)

//sk unused
//template<class T, unsigned Width, SKLIB_INTERNAL_ENABLE_IF_INT(T)>
//inline constexpr T bits_data_cap_v = sklib::priv::bits_data_cap<T, Width>();
//
// template<class T, unsigned Width, SKLIB_INTERNAL_ENABLE_IF_INT(T)>
//inline constexpr T bits_data_high_1_v = sklib::priv::bits_data_high_1<T, Width>();

template<class T, unsigned Width, SKLIB_INTERNAL_ENABLE_IF_INT(T)>
inline constexpr T bits_data_mask_v = bits_data_high_1_v<T, Width> | T(bits_data_high_1_v<T, Width>-1);

// bits for the type

SKLIB_TEMPLATE_IF_UINT(T) inline constexpr T bits_high_1_v = bits_data_high_1_v<T, bits_width_v<T>>;
SKLIB_TEMPLATE_IF_UINT(T) inline constexpr T bits_mask_v = bits_high_1_v<T> | T(bits_high_1_v<T> - 1);

SKLIB_TEMPLATE_IF_UINT(T) inline constexpr T bits_low_half_v = (T(1) << (bits_width_v<T>/2)) - 1;
SKLIB_TEMPLATE_IF_UINT(T) inline constexpr T bits_high_half_v = (bits_low_half_v<T> << (bits_width_v<T>/2));

inline constexpr uint8_t OCTET_MASK         = bits_mask_v<uint8_t>;
inline constexpr size_t  OCTET_ADDRESS_SPAN = OCTET_MASK + 1;

#endif

/* //sk delete
// bits by count as function argument

template<class T> constexpr T bits_data_cap(unsigned width)
{
    return (T(1) << width);
}
SKLIB_TEMPLATE_IF_INT(T) constexpr T bits_data_high_1(unsigned width)
{
    return (T(1) << (width-1));
}
SKLIB_TEMPLATE_IF_INT(T) constexpr T bits_data_mask(unsigned width)
{
    const auto A = bits_data_high_1<T>(width);
    return A | (A-1);
}
*/

}; // namespace sklib

#endif // SKLIB_INCLUDED_TYPES_INTBITS_HPP

